#include "r_archive.h"

static int copy_data(struct archive* ar, struct archive* aw) {
  int r;
  const void* buff;
  size_t size;
  int64_t offset;

  for (;;) {
    r = call(archive_read_data_block, ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return (ARCHIVE_OK);
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

[[cpp11::register]] void archive_extract_(
    const cpp11::sexp& connection,
    cpp11::sexp file,
    cpp11::strings options,
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

  for (R_xlen_t index = 1;; ++index) {
    res = call(archive_read_next_header, a, &entry);
    if (res == ARCHIVE_EOF)
      break;
    const char* filename = archive_entry_pathname(entry);
    if (file == R_NilValue ||
        (!file_indexes.empty() && any_matches(index, file_indexes)) ||
        (!file_names.empty() && any_matches(filename, file_names))) {
      call(archive_write_header, ext, entry);
      copy_data(a, ext);
      call(archive_write_finish_entry, ext);
    }
  }
  call(archive_read_close, a);
  call(archive_read_free, a);
  call(archive_write_close, ext);
  call(archive_write_free, ext);
}
