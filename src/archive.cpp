#include "r_archive.h"

#include <locale>
#include <vector>

using namespace cpp11::literals;

[[cpp11::register]] cpp11::sexp
archive_(const std::string& path, cpp11::strings options) {
  // Set a UTF-8 locale
  std::string old_locale(std::setlocale(LC_ALL, ""));
  if (nullptr == std::setlocale(LC_ALL, "en_US.UTF-8")) {
    cpp11::warning(
        "Unable to set a UTF-8 locale!\n  archive paths with unicode "
        "characters may not be handled properly");
  }

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

  call(archive_read_open_filename, a, path.c_str(), 10240);
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

  out.attr("path") = path;

  std::setlocale(LC_ALL, old_locale.c_str());

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
