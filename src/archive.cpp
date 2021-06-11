#include "r_archive.h"

#include <vector>

using namespace cpp11::literals;

[[cpp11::register]] cpp11::sexp archive_metadata(const std::string& path) {
  std::vector<std::string> paths;
  std::vector<__LA_INT64_T> sizes;
  std::vector<time_t> dates;

  struct archive* a;
  struct archive_entry* entry;
  int r;

  a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);

  r = archive_read_set_options(a, "read_concatenated_archives");
  if (r != ARCHIVE_OK) {
    Rf_error(archive_error_string(a));
  }
  r = archive_read_open_filename(a, path.c_str(), 10240);
  if (r != ARCHIVE_OK) {
    cpp11::stop(archive_error_string(a));
  }
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    paths.push_back(archive_entry_pathname(entry));
    sizes.push_back(archive_entry_size(entry));
    dates.push_back(archive_entry_mtime(entry));
    archive_read_data_skip(a);
  }
  r = archive_read_free(a);
  if (r != ARCHIVE_OK) {
    cpp11::stop(archive_error_string(a));
  }

  static auto as_tibble = cpp11::package("tibble")["as_tibble"];
  cpp11::writable::doubles d(dates);
  d.attr("class") = {"POSIXct", "POSIXt"};

  cpp11::writable::list out(
      {"path"_nm = paths, "size"_nm = sizes, "date"_nm = d});

  out.attr("path") = path;

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
