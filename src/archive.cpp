#include "r_archive.h"
#include <Rcpp.h>

// [[Rcpp::export]]
Rcpp::List archive_metadata(const std::string& path) {
  std::vector<std::string> paths;
  std::vector<__LA_INT64_T> sizes;
  std::vector<time_t> dates;

  struct archive* a;
  struct archive_entry* entry;
  int r;

  a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  r = archive_read_open_filename(a, path.c_str(), 10240);
  if (r != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(a));
  }
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    paths.push_back(archive_entry_pathname(entry));
    sizes.push_back(archive_entry_size(entry));
    dates.push_back(archive_entry_mtime(entry));
    archive_read_data_skip(a);
  }
  r = archive_read_free(a);
  if (r != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(a));
  }

  static Rcpp::Function as_tibble(
      "as_tibble", Rcpp::Environment::namespace_env("tibble"));
  Rcpp::NumericVector d = Rcpp::wrap(dates);
  d.attr("class") = Rcpp::CharacterVector::create("POSIXct", "POSIXt");

  Rcpp::List out = as_tibble(Rcpp::List::create(
      Rcpp::_["path"] = paths, Rcpp::_["size"] = sizes, Rcpp::_["date"] = d));

  out.attr("path") = path;

  return out;
}

// [[Rcpp::export]]
Rcpp::IntegerVector archive_filters() {
  Rcpp::IntegerVector out = Rcpp::IntegerVector::create(
      Rcpp::_["none"] = ARCHIVE_FILTER_NONE,
      Rcpp::_["gzip"] = ARCHIVE_FILTER_GZIP,
      Rcpp::_["bzip2"] = ARCHIVE_FILTER_BZIP2,
      Rcpp::_["compress"] = ARCHIVE_FILTER_COMPRESS,
      Rcpp::_["lzma"] = ARCHIVE_FILTER_LZMA,
      Rcpp::_["xz"] = ARCHIVE_FILTER_XZ,
      Rcpp::_["uuencode"] = ARCHIVE_FILTER_UU,
      Rcpp::_["lzip"] = ARCHIVE_FILTER_LZIP
#if ARCHIVE_VERSION_NUMBER >= 3001000
      ,
      Rcpp::_["lrzip"] = ARCHIVE_FILTER_LRZIP,
      Rcpp::_["lzop"] = ARCHIVE_FILTER_LZOP,
      Rcpp::_["grzip"] = ARCHIVE_FILTER_GRZIP
#endif

#if ARCHIVE_VERSION_NUMBER >= 3002000
      ,
      Rcpp::_["lz4"] = ARCHIVE_FILTER_LZ4
#endif
      );
  return out;
}

// [[Rcpp::export]]
Rcpp::IntegerVector archive_formats() {
  Rcpp::IntegerVector out = Rcpp::IntegerVector::create(
      Rcpp::_["7zip"] = ARCHIVE_FORMAT_7ZIP,
      Rcpp::_["arbsd"] = ARCHIVE_FORMAT_AR,
      Rcpp::_["cab"] = ARCHIVE_FORMAT_CAB,
      Rcpp::_["cpio"] = ARCHIVE_FORMAT_CPIO,
      Rcpp::_["iso9660"] = ARCHIVE_FORMAT_ISO9660,
      Rcpp::_["lha"] = ARCHIVE_FORMAT_LHA,
      Rcpp::_["mtree"] = ARCHIVE_FORMAT_MTREE,
      Rcpp::_["shar"] = ARCHIVE_FORMAT_SHAR,
      Rcpp::_["rar"] = ARCHIVE_FORMAT_RAR,
      Rcpp::_["tar"] = ARCHIVE_FORMAT_TAR,
      Rcpp::_["xar"] = ARCHIVE_FORMAT_XAR,
      Rcpp::_["zip"] = ARCHIVE_FORMAT_ZIP
#if ARCHIVE_VERSION_NUMBER >= 3002000
      ,

      Rcpp::_["warc"] = ARCHIVE_FORMAT_WARC
#endif
      );
  return out;
}

// [[Rcpp::export]]
std::string libarchive_version() {
  std::string str = archive_version_string();

  // Remove 'libarchive '
  return str.substr(str.find_last_of(' ') + 1);
}
