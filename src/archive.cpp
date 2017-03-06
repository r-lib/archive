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

static Rboolean rchive_open(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;

  if ((r->last_response = archive_read_open_filename(r->ar, r->filename, 10240)) != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(r->ar));
  }
  if ((r->last_response = archive_read_next_header(r->ar, &r->entry)) != ARCHIVE_OK) {
    Rcpp::stop(archive_error_string(r->ar));
  }
  size_t entries = archive_entry_size(r->entry);
  if (entries <= 0) {
    Rcpp::stop("empty entry");
  }
  r->size = 0;
  r->has_more = 1;
  con->isopen = TRUE;

  return TRUE;
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
  free(r->filename);
  free(r);
}

void copy_data(rchive *r) {
  R_CheckUserInterrupt();
  size_t size;
  la_int64_t offset;
  while(r->last_response != ARCHIVE_EOF) {
    r->last_response = archive_read_data_block(r->ar, (const void **) &r->buf, &size, &offset);
    if (r->last_response == ARCHIVE_EOF) {
      r->has_more = 0;
      return;
    }
    if (r->last_response != ARCHIVE_OK) {
      Rcpp::stop(archive_error_string(r->ar));
    }
  }
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
SEXP read_connection(const std::string filename, size_t sz = 16384) {
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection(filename.c_str(), "r", "archive", &con));

  /* Setup archive */
  rchive *r = (rchive *) malloc(sizeof(rchive));
  r->limit = sz;
  r->buf = (char *) malloc(r->limit);

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
