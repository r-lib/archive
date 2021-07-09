#include "r_archive.h"

/* Read archives
 *
 * The custom R connection code was adapted from curl package by Jeroen Ooms
 * Source:
 * https://github.com/cran/curl/blob/3897ba5203dee940e2ce40ac23a0d1106da93df6/src/curl.c
 *
 * The libarchive code from libarchive examples
 * Source: https://github.com/libarchive/libarchive/wiki/Examples
 */

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

  call(
      archive_read_open_filename,
      con,
      r->archive_filename.c_str(),
      r->buf.size());

  /* Find entry to extract */
  while (archive_read_next_header(r->ar, &r->entry) == ARCHIVE_OK) {
    const char* str = archive_entry_pathname(r->entry);
    if (is_raw_format || strcmp(r->filename.c_str(), str) == 0) {
      r->has_more = 1;
      con->isopen = TRUE;
      push(r);
      return TRUE;
    }
    call(archive_read_data_skip, con);
  }

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
    const std::string& archive_filename,
    const std::string& filename,
    const std::string& mode,
    cpp11::integers format,
    cpp11::integers filters,
    cpp11::strings options,
    size_t sz = 16384) {
  Rconnection con;

  std::string desc = archive_filename + '[' + filename + ']';
  SEXP rc =
      PROTECT(new_connection(desc.c_str(), mode.c_str(), "archive_read", &con));

  /* Setup archive */
  rchive* r = new rchive;
  r->buf.resize(sz);
  r->size = 0;
  r->cur = NULL;

  r->archive_filename = archive_filename;

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

  r->filename = filename;

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
