#include "r_archive.h"

#include <vector>

using namespace cpp11::literals;

[[cpp11::register]] cpp11::sexp
archive_(cpp11::sexp connection, cpp11::strings options, cpp11::strings password) {

  local_utf8_locale ll;

  std::vector<std::string> paths;
  std::vector<__LA_INT64_T> sizes;
  std::vector<time_t> dates;

  struct archive* a;
  struct archive_entry* entry;

  a = archive_read_new();
  call(archive_read_support_filter_all, a);
  call(archive_read_support_format_all, a);

  if (options.size() > 0) {
    call(archive_read_set_options, a, std::string(options[0]).c_str());
  }
  if (!cpp11::is_na(password[0])) {
    call(archive_read_add_passphrase, a, std::string(password[0]).c_str());
  }

  std::unique_ptr<input_data> r(new input_data);
  r->buf.resize(16384);
  r->connection = connection;

  call(archive_read_set_read_callback, a, input_read);
  call(archive_read_set_close_callback, a, input_close);
  static auto isSeekable = cpp11::package("base")["isSeekable"];
  if (isSeekable(connection)) {
    call(archive_read_set_seek_callback, a, input_seek);
  }
  call(archive_read_set_callback_data, a, r.get());
  call(archive_read_open1, a);

  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    paths.push_back(archive_entry_pathname(entry));
    sizes.push_back(archive_entry_size(entry));
    dates.push_back(archive_entry_mtime(entry));
    call(archive_read_data_skip, a);
  }
  call(archive_read_free, a);

  static auto as_tibble = cpp11::package("tibble")["as_tibble"];
  cpp11::writable::doubles d(dates);
  d.attr("class") = {"POSIXct", "POSIXt"};

  cpp11::writable::list out(
      {"path"_nm = paths, "size"_nm = sizes, "date"_nm = d});

  // out.attr("connection") = static_cast<SEXP>(connection);

  return as_tibble(out);
}

[[cpp11::register]] cpp11::integers archive_filters() {
  cpp11::writable::integers out({
    "none"_nm = ARCHIVE_FILTER_NONE, "gzip"_nm = ARCHIVE_FILTER_GZIP,
    "bzip2"_nm = ARCHIVE_FILTER_BZIP2, "compress"_nm = ARCHIVE_FILTER_COMPRESS,
    "lzma"_nm = ARCHIVE_FILTER_LZMA, "xz"_nm = ARCHIVE_FILTER_XZ,
    "uuencode"_nm = ARCHIVE_FILTER_UU, "lzip"_nm = ARCHIVE_FILTER_LZIP
#if ARCHIVE_VERSION_NUMBER >= 3001000
        ,
    "lrzip"_nm = ARCHIVE_FILTER_LRZIP, "lzop"_nm = ARCHIVE_FILTER_LZOP,
    "grzip"_nm = ARCHIVE_FILTER_GRZIP
#endif

#if ARCHIVE_VERSION_NUMBER >= 3002000
        ,
    "lz4"_nm = ARCHIVE_FILTER_LZ4
#endif

#if ARCHIVE_VERSION_NUMBER >= 3003003
        ,
    "zstd"_nm = ARCHIVE_FILTER_ZSTD
#endif
  });
  return out;
}

[[cpp11::register]] cpp11::integers archive_formats() {
  cpp11::writable::integers out({
    "7zip"_nm = ARCHIVE_FORMAT_7ZIP, "cab"_nm = ARCHIVE_FORMAT_CAB,
    "cpio"_nm = ARCHIVE_FORMAT_CPIO, "iso9660"_nm = ARCHIVE_FORMAT_ISO9660,
    "lha"_nm = ARCHIVE_FORMAT_LHA, "mtree"_nm = ARCHIVE_FORMAT_MTREE,
    "shar"_nm = ARCHIVE_FORMAT_SHAR, "rar"_nm = ARCHIVE_FORMAT_RAR,
    "raw"_nm = ARCHIVE_FORMAT_RAW, "tar"_nm = ARCHIVE_FORMAT_TAR,
    "xar"_nm = ARCHIVE_FORMAT_XAR, "zip"_nm = ARCHIVE_FORMAT_ZIP
#if ARCHIVE_VERSION_NUMBER >= 3002000
        ,
    "warc"_nm = ARCHIVE_FORMAT_WARC
#endif
  });
  return out;
}

[[cpp11::register]] std::string libarchive_version_() {
  std::string str = archive_version_string();

  // Remove 'libarchive '
  return str.substr(str.find_last_of(' ') + 1);
}

[[cpp11::register]] const char* libarchive_zlib_version_() {
#if ARCHIVE_VERSION_NUMBER >= 3002000
  const char* version = archive_zlib_version();
  return version ? version : "";
#else
  return "";
#endif
}

[[cpp11::register]] const char* libarchive_liblzma_version_() {
#if ARCHIVE_VERSION_NUMBER >= 3002000
  const char* version = archive_liblzma_version();
  return version ? version : "";
#else
  return "";
#endif
}

[[cpp11::register]] const char* libarchive_bzlib_version_() {
#if ARCHIVE_VERSION_NUMBER >= 3002000
  const char* version = archive_bzlib_version();
  return version ? version : "";
#else
  return "";
#endif
}

[[cpp11::register]] const char* libarchive_liblz4_version_() {
#if ARCHIVE_VERSION_NUMBER >= 3002000
  const char* version = archive_liblz4_version();
  return version ? version : "";
#else
  return "";
#endif
}

[[cpp11::register]] const char* libarchive_libzstd_version() {
#if ARCHIVE_VERSION_NUMBER >= 3003003
  const char* version = archive_libzstd_version();
  return version ? version : "";
#else
  return "";
#endif
}
