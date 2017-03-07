/* Read archives
 *
 * The custom R connection code was adapted from curl package by Jeroen Ooms
 * Source: https://github.com/cran/curl/blob/3897ba5203dee940e2ce40ac23a0d1106da93df6/src/curl.c
 *
 * The libarchive code from libarchive examples
 * Source: https://github.com/libarchive/libarchive/wiki/Examples
 */
#include <archive.h>
#include <archive_entry.h>
#include <Rcpp.h>

#define class class_name
#define private private_ptr
#include <R_ext/Connections.h>
#undef class
#undef private

#if R_CONNECTIONS_VERSION != 1
#error "Missing or unsupported connection API in R"
#endif

#if defined(R_VERSION) && R_VERSION >= R_Version(3, 3, 0)
Rconnection get_connection(SEXP con) {
  return R_GetConnection(con);
}
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
  char* path;
  char* filename;
  char* buf;
  char* cur;
  archive *ar;
  archive_entry *entry;
  int last_response;
  int has_more;
  size_t limit;
  size_t size;
} rchive;

static size_t pop(void *target, size_t max, rchive *r){
  size_t copy_size = min(r->size, max);
  memcpy(target, r->cur, copy_size);
  r->cur += copy_size;
  r->size -= copy_size;
  return copy_size;
}

void copy_data(rchive *r) {
  R_CheckUserInterrupt();
  la_int64_t offset;
  if(r->last_response != ARCHIVE_EOF) {

    /* move existing data to front of buffer (if any) */
    memcpy(r->buf, r->cur, r->size);

    r->last_response = archive_read_data_block(r->ar, (const void **) &r->buf, &r->size, &offset);
    if (r->last_response == ARCHIVE_EOF) {
      r->has_more = 0;
      return;
    }
    if (r->last_response != ARCHIVE_OK) {
      Rcpp::stop(archive_error_string(r->ar));
    }
    r->cur = r->buf;
  }
}

static Rboolean rchive_open(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;

  if ((r->last_response = archive_read_open_filename(r->ar, r->path, 10240)) != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(r->ar));
  }
  if ((r->last_response = archive_read_next_header(r->ar, &r->entry)) != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(r->ar));
  }

  /* Find entry to extract */
  while (archive_read_next_header(r->ar, &r->entry) == ARCHIVE_OK) {
    const char * str = archive_entry_pathname(r->entry);
    if (strcmp(r->filename, str) == 0) {
      r->size = archive_entry_size(r->entry);
      r->cur = r->buf;
      r->has_more = 1;
      con->isopen = TRUE;
      copy_data(r);
      return TRUE;
    }
    archive_read_data_skip(r->ar);
  }
  Rcpp::stop("'%s' not found in archive", r->filename);
  return FALSE;
}

void rchive_close(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;
  archive_read_close(r->ar);

  con->isopen = FALSE;
}

void rchive_destroy(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;

  /* free the handle connection */
  archive_read_free(r->ar);
  free(r->buf);
  free(r->path);
  free(r->filename);
  free(r);
}

/* Support for readBin() */
static size_t rchive_read(void *target, size_t sz, size_t ni, Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;
  size_t size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, size, r);
  while((size > total_size) && r->has_more) {
    copy_data(r);
    total_size += pop((char*)target + total_size, (size-total_size), r);
  }
  con->incomplete = (Rboolean) r->has_more;
  return total_size;
}

// [[Rcpp::export]]
SEXP read_connection(const std::string & path, const std::string & filename, size_t sz = 16384) {
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection(std::string(path + '[' + filename + ']').c_str(), "r", "archive", &con));

  /* Setup archive */
  rchive *r = (rchive *) malloc(sizeof(rchive));
  r->limit = sz;
  r->buf = (char *) malloc(r->limit);

  r->path = (char *) malloc(strlen(path.c_str()) + 1);
  strcpy(r->path, path.c_str());

  r->filename = (char *) malloc(strlen(filename.c_str()) + 1);
  strcpy(r->filename, filename.c_str());

  r->ar = archive_read_new();
  r->last_response = archive_read_support_filter_all(r->ar);
  r->last_response = archive_read_support_format_all(r->ar);

  /* set connection properties */
  con->incomplete = TRUE;
  con->private_ptr = r;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = FALSE;
  con->UTF8out = FALSE;
  con->open = rchive_open;
  con->close = rchive_close;
  con->destroy = rchive_destroy;
  con->read = rchive_read;

  UNPROTECT(1);
  return rc;
}

// [[Rcpp::export]]
Rcpp::List archive_metadata(const std::string & path) {
  std::vector<std::string> paths;
  std::vector<la_int64_t> sizes;
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

