#include "r_archive.h"
#include <cli/progress.h>

const char* const pb_format =
    "{cli::pb_spin} %zu extracted | {cli::pb_current_bytes} "
    "({cli::pb_rate_bytes}) | "
    "{cli::pb_elapsed}";

static int copy_data(
    struct archive* ar,
    struct archive* aw,
    SEXP progress_bar,
    size_t& total_read,
    size_t num_extracted) {
  int r;
  const void* buff;
  size_t size;
  int64_t offset;

  for (;;) {
    r = call(archive_read_data_block, ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return (ARCHIVE_OK);
    }
    total_read += size;

    if (CLI_SHOULD_TICK) {
      cli_progress_set_format(progress_bar, pb_format, num_extracted);

      cli_progress_set(progress_bar, total_read);
    }

    call(archive_write_data_block, aw, buff, size, offset);
  }
}

template <typename C> std::vector<R_xlen_t> as_file_index(const C& in) {
  std::vector<R_xlen_t> out;
  out.reserve(in.size());
  for (R_xlen_t value : in) {
    out.push_back(value);
  }
  return out;
}

template <typename T, typename C>
bool any_matches(const T& needle, const std::vector<C>& haystack) {
  for (const C& n : haystack) {
    if (n == needle) {
      return true;
    }
  }
  return false;
}

/* From
https://github.com/libarchive/libarchive/blob/0fd2ed25d78e9f4505de5dcb6208c6c0ff8d2edb/tar/util.c#L338-L375
*/
static const char* strip_components(const char* p, int elements) {
  /* Skip as many elements as necessary. */
  while (elements > 0) {
    switch (*p++) {
    case '/':
#if defined(_WIN32)
    case '\\': /* Support \ path sep on Windows ONLY. */
#endif
      elements--;
      break;
    case '\0':
      /* Path is too short, skip it. */
      return (NULL);
    }
  }

  /* Skip any / characters.  This handles short paths that have
   * additional / termination.  This also handles the case where
   * the logic above stops in the middle of a duplicate //
   * sequence (which would otherwise get converted to an
   * absolute path). */
  for (;;) {
    switch (*p) {
    case '/':
#if defined(_WIN32)
    case '\\': /* Support \ path sep on Windows ONLY. */
#endif
      ++p;
      break;
    case '\0':
      return (NULL);
    default:
      return (p);
    }
  }
}

[[cpp11::register]] cpp11::strings archive_extract_(
    const cpp11::sexp& connection,
    cpp11::sexp file,
    int num_strip_components,
    cpp11::strings options,
    cpp11::r_string password,
    size_t sz = 16384) {
  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;
  int flags;
  int res;

  local_utf8_locale ll;

  std::unique_ptr<input_data> r(new input_data);
  r->buf.resize(16384);
  r->connection = connection;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  call(archive_read_support_format_all, a);
  call(archive_read_support_filter_all, a);

  if (options.size() > 0) {
    call(archive_read_set_options, a, std::string(options[0]).c_str());
  }

  if (!cpp11::is_na(password)) {
    call(archive_read_add_passphrase, a, std::string(password).c_str());
  }

  call(archive_read_set_read_callback, a, input_read);
  call(archive_read_set_close_callback, a, input_close);
  static auto isSeekable = cpp11::package("base")["isSeekable"];
  if (isSeekable(connection)) {
    call(archive_read_set_seek_callback, a, input_seek);
  }
  call(archive_read_set_callback_data, a, r.get());
  call(archive_read_open1, a);

  ext = archive_write_disk_new();
  call(archive_write_disk_set_options, ext, flags);
#ifndef __MINGW32__
  /* set_standard_lookup is not available on windows */
  call(archive_write_disk_set_standard_lookup, ext);
#endif

  std::vector<R_xlen_t> file_indexes;
  std::vector<std::string> file_names;

  if (TYPEOF(file) == INTSXP) {
    file_indexes = as_file_index(cpp11::integers(file));
  } else if (TYPEOF(file) == REALSXP) {
    file_indexes = as_file_index(cpp11::doubles(file));
  } else if (TYPEOF(file) == STRSXP) {
    file_names = cpp11::as_cpp<std::vector<std::string>>(file);
  }

  using namespace cpp11::literals;

  cpp11::sexp progress_bar(cli_progress_bar(NA_INTEGER, R_NilValue));

  size_t total_read = 0;

  size_t num_extracted = 0;

  cpp11::writable::strings extracted_files;

  for (R_xlen_t index = 1;; ++index) {
    res = call(archive_read_next_header, a, &entry);
    if (res == ARCHIVE_EOF) {
      break;
    }
    const char* filename = archive_entry_pathname(entry);
    const char* original_filename = filename;
    if (num_strip_components > 0) {
      filename = strip_components(filename, num_strip_components);
      if (filename == nullptr) {
        continue;
      }
    }

    if (file == R_NilValue ||
        (!file_indexes.empty() && any_matches(index, file_indexes)) ||
        (!file_names.empty() && any_matches(filename, file_names))) {
      extracted_files.push_back(filename);

      if (filename != original_filename) {
        archive_entry_copy_pathname(entry, filename);
      }

      call(archive_write_header, ext, entry);
      copy_data(a, ext, progress_bar, total_read, num_extracted);
      call(archive_write_finish_entry, ext);

      ++num_extracted;
    }
  }

  cli_progress_done(progress_bar);

  call(archive_read_close, a);
  call(archive_read_free, a);
  call(archive_write_close, ext);
  call(archive_write_free, ext);

  return extracted_files;
}
