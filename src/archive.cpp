#include "r_archive.h"
#include <Rcpp.h>

// [[Rcpp::export]]
Rcpp::List archive_metadata(const std::string & path) {
  std::vector<std::string> paths;
  std::vector<__LA_INT64_T> sizes;
  std::vector<time_t> dates;

  struct archive *a;
  struct archive_entry *entry;
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

  static Rcpp::Function as_tibble("as_tibble", Rcpp::Environment::namespace_env("tibble"));
  Rcpp::NumericVector d = Rcpp::wrap(dates);
  d.attr("class") = Rcpp::CharacterVector::create("POSIXct", "POSIXt");

  Rcpp::List out = as_tibble(Rcpp::List::create(
      Rcpp::_["path"] = paths,
      Rcpp::_["size"] = sizes,
      Rcpp::_["date"] = d));

  out.attr("path") = path;

  return out;
}
