#include "r_archive.h"

/* Define BSWAP_32 on Big Endian systems */
#ifdef WORDS_BIGENDIAN
#if (defined(__sun) && defined(__SVR4))
#include <sys/byteorder.h>
#elif (defined(__APPLE__) && defined(__ppc__) || defined(__ppc64__))
#include <libkern/OSByteOrder.h>
#define BSWAP_32 OSSwapInt32
#elif (defined(__OpenBSD__))
#define BSWAP_32(x) swap32(x)
#elif (defined(__NetBSD__))
#include <sys/types.h>
#include <machine/bswap.h>
#define BSWAP_32(x) bswap32(x)
#elif (defined(__GLIBC__))
#include <byteswap.h>
#define BSWAP_32(x) bswap_32(x)
#elif (defined(_AIX))
#define BSWAP_32(x) __builtin_bswap32(x)
#endif
#endif

/* Read archives
 *
 * The custom R connection code was adapted from curl package by Jeroen Ooms
 * Source:
 * https://github.com/cran/curl/blob/3897ba5203dee940e2ce40ac23a0d1106da93df6/src/curl.c
 *
 * The libarchive code from libarchive examples
 * Source: https://github.com/libarchive/libarchive/wiki/Examples
 */

ssize_t input_read(struct archive* a, void* client_data, const void** buff) {
  struct input_data* data = static_cast<input_data*>(client_data);
  *buff = data->buf.data();
  return read_connection(data->connection, data->buf.data(), data->buf.size());
}

int64_t
input_seek(struct archive*, void* client_data, int64_t offset, int whence) {
  struct input_data* data = static_cast<input_data*>(client_data);
  static auto seek = cpp11::package("base")["seek"];
  if(offset != (double)offset){
      REprintf("As R does not support int64, tried to use double, but the conversion didn't went well, error will be raised. original=%lld, converted=%lld\n",offset,(int64_t)((double)offset));
  }

  seek(
      data->connection,
      (double)offset,
      whence == SEEK_END ? "end" : whence == SEEK_CUR ? "current" : "start");
  /* need to call seek again to get the current position */
  int64_t value = cpp11::as_cpp<int64_t>(seek(data->connection));

  return value;
}

int input_close(struct archive* a, void* client_data) {
  struct input_data* data = static_cast<input_data*>(client_data);
  static auto close = cpp11::package("base")["close"];

  close(data->connection);
  return (ARCHIVE_OK);
}

bool entry_matches(const std::string& str, archive_entry* entry) {
  if (str.empty()) {
    return false;
  }

  const char* pathname = archive_entry_pathname(entry);
  return str == pathname;
}

static Rboolean rchive_read_open(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  local_utf8_locale ll;

  r->ar = archive_read_new();

  bool is_raw_format = r->format == ARCHIVE_FORMAT_RAW;

  con->text = strchr(con->mode, 'b') ? FALSE : TRUE;

/* explicit setting of the format and filters is not available until
 * libarchive version 3.1.0
 */
#if ARCHIVE_VERSION_NUMBER >= 3001000
  if (r->filters[0] == -1) {
    call(archive_read_support_filter_all, con);
  } else {
    for (int i = 0; i < FILTER_MAX && r->filters[i] != -1; ++i) {
      call(archive_read_append_filter, con, r->filters[i]);
    }
  }

  if (r->format == -1) {
    call(archive_read_support_format_all, con);
  } else if (is_raw_format) {
    call(archive_read_support_format_raw, con);
  } else {
    call(archive_read_set_format, con, r->format);
  }
#else
  call(archive_read_support_filter_all, con);
  call(archive_read_support_format_all, con);
#endif

  if (!r->options.empty()) {
    call(archive_read_set_options, con, r->options.c_str());
  }

  static auto open = cpp11::package("base")["open"];
  static auto isOpen = cpp11::package("base")["isOpen"];
  if (!isOpen(r->input.connection)) {
    open(r->input.connection, "rb");
  }
  call(archive_read_set_read_callback, r->ar, input_read);
  call(archive_read_set_close_callback, r->ar, input_close);
  static auto isSeekable = cpp11::package("base")["isSeekable"];
  if (isSeekable(r->input.connection)) {
    call(archive_read_set_seek_callback, r->ar, input_seek);
  }
  call(archive_read_set_callback_data, r->ar, &r->input);
  call(archive_read_open1, r->ar);

  /* Find entry to extract */
  int file_offset = -1;
  std::string file;

  if (TYPEOF(r->file) == INTSXP || TYPEOF(r->file) == REALSXP) {
    file_offset = cpp11::as_cpp<int>(r->file) - 1;
  } else {
    file = cpp11::as_cpp<std::string>(r->file);
  }

  int itr = 0;
  int res;
  while ((res = archive_read_next_header(r->ar, &r->entry)) == ARCHIVE_OK) {
    if (is_raw_format || entry_matches(file, r->entry) || itr == file_offset) {
      r->has_more = 1;
      con->isopen = TRUE;
      push(r);
      return TRUE;
    }
    call(archive_read_data_skip, con);
    ++itr;
  }

  con->isopen = FALSE;
  const char* msg = archive_error_string(r->ar);
  Rf_errorcall(R_NilValue, "%s", msg);
  return FALSE;
}

void rchive_read_close(Rconnection con) {
  call(archive_read_close, con);

  con->isopen = FALSE;
  con->incomplete = FALSE;
}

void rchive_read_destroy(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  /* free the handle connection */
  call(archive_read_free, con);

  delete r;
}

/* Support for readBin() */
static size_t rchive_read(void* target, size_t sz, size_t ni, Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;
  size_t size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, size, r);
  while ((size > total_size) && r->has_more) {
    push(r);
    total_size += pop((char*)target + total_size, (size - total_size), r);
  }
  con->incomplete = (Rboolean)r->has_more;
  return total_size;
}

/* https://github.com/jeroen/curl/blob/102eb33288c853e0b3d4344fa1725388f606cecc/src/curl.c#L145
 */
/* naive implementation of readLines */
static int rchive_fgetc(Rconnection con) {
  int x = 0;
#ifdef WORDS_BIGENDIAN
  return rchive_read(&x, 1, 1, con) ? BSWAP_32(x) : R_EOF;
#else
  return rchive_read(&x, 1, 1, con) ? x : R_EOF;
#endif
}

[[cpp11::register]] SEXP archive_read_(
    const cpp11::sexp connection,
    const cpp11::sexp file,
    const std::string& description,
    const std::string& mode,
    cpp11::integers format,
    cpp11::integers filters,
    cpp11::strings options,
    size_t sz = 16384) {
  Rconnection con;

  SEXP rc = PROTECT(
      new_connection(description.c_str(), mode.c_str(), "archive_read", &con));

  /* Setup archive */
  rchive* r = new rchive;
  r->buf.resize(sz);
  r->size = 0;
  r->cur = NULL;

  r->input.connection = connection;
  r->input.buf.resize(sz);

  if (options.size() > 0) {
    r->options = options[0];
  }

  r->format = format.size() == 0 ? -1 : format[0];

  /* Initialize filters */
  if (filters.size() > FILTER_MAX) {
    cpp11::stop("Cannot use more than %i filters", FILTER_MAX);
  }
  for (int i = 0; i < FILTER_MAX; ++i) {
    r->filters[i] = -1;
  }
  for (int i = 0; i < filters.size(); ++i) {
    r->filters[i] = filters[i];
  }

  r->file = file;

  /* set connection properties */
  con->incomplete = TRUE;
  con->private_ptr = r;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->UTF8out = FALSE;
  con->open = rchive_read_open;
  con->close = rchive_read_close;
  con->destroy = rchive_read_destroy;
  con->read = rchive_read;
  con->fgetc = rchive_fgetc;
  con->fgetc_internal = rchive_fgetc;
  con->text = strchr(con->mode, 'b') ? FALSE : TRUE;

  UNPROTECT(1);
  return rc;
}
