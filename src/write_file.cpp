#include "r_archive.h"
#include <Rcpp.h>
#include <fcntl.h>

#if ARCHIVE_VERSION_NUMBER >= 3001000
typedef struct {
  archive *ar;
  archive_entry *entry;
  char* filename;
  SEXP filters;
} file;

/* callback function to store received data */
static size_t file_write_data(const void *contents, size_t sz, size_t n, Rconnection ctx) {
  file* r = (file*) ctx->private_ptr;

  size_t realsize = sz * n;
  archive_write_data(r->ar, contents, realsize);

  return n;
}

static Rboolean file_write_open(Rconnection con) {
  file *r = (file *) con->private_ptr;

  r->ar = archive_write_new();

  SEXP filters = r->filters;
  for (int i = 0; i < Rf_length(filters); ++i) {
    int filter = INTEGER(filters)[i];
    int ret = archive_write_add_filter(r->ar, filter);
    if (ret == ARCHIVE_FATAL)  {
      Rf_error("%i", filter);
      Rf_error(archive_error_string(r->ar));
    }
  }
  archive_write_set_format_raw(r->ar);
  archive_write_open_filename(r->ar, r->filename);

  r->entry = archive_entry_new();

  archive_entry_set_pathname(r->entry, r->filename);
  archive_entry_set_filetype(r->entry, AE_IFREG);
  archive_entry_set_perm(r->entry, 0644);
  archive_write_header(r->ar, r->entry);

  con->isopen = TRUE;
  return TRUE;
}

/* This function closes the temporary scratch file, then writes the actual
 * archive file based on the archive filename given and then unlinks the
 * scratch file */
void file_write_close(Rconnection con) {
  file *r = (file *) con->private_ptr;

  archive_entry_free(r->entry);
  archive_write_close(r->ar);
  archive_write_free(r->ar);
}

void file_write_destroy(Rconnection con) {
  file *r = (file *) con->private_ptr;

  /* free the handle connection */
  free(r->filename);
  free(r);
}
#endif

// Get a connection to a single non-archive file, optionally with one or more
// filters.
// [[Rcpp::export]]
SEXP write_file_connection(const std::string & filename, SEXP filters) {
#if ARCHIVE_VERSION_NUMBER < 3001000
  Rcpp::stop("This functionality is only available with libarchive >= 3.1.0");
#else
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection("file_output", "wb", "archive", &con));

  /* Setup file */
  file *r = (file *) malloc(sizeof(file));

  r->filename = (char *) malloc(strlen(filename.c_str()) + 1);
  strcpy(r->filename, filename.c_str());

  r->filters = filters;

  /* set connection properties */
  con->incomplete = TRUE;
  con->private_ptr = r;
  con->canread = FALSE;
  con->canseek = FALSE;
  con->canwrite = TRUE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = FALSE;
  con->open = file_write_open;
  con->close = file_write_close;
  con->destroy = file_write_destroy;
  con->write = file_write_data;

  UNPROTECT(1);
  return rc;
#endif
}

// Write files already on disk to a new archive
// [[Rcpp::export]]
SEXP write_files(const std::string & archive_filename, Rcpp::CharacterVector files, int format, Rcpp::NumericVector filter, size_t sz = 16384) {

#if ARCHIVE_VERSION_NUMBER < 3001000
  Rcpp::stop("This functionality is only available with libarchive >= 3.1.0");
#else
  struct archive *a;
  struct archive_entry *entry;
  struct stat st;
  char buff[8192];
  int len;
  int fd;
  int response;

  a = archive_write_new();

  response = archive_write_set_format(a, format);
  if (response != ARCHIVE_OK) {
    Rf_error(archive_error_string(a));
  }

  for (int i = 0;i < filter.length();++i) {
    response = archive_write_add_filter(a, filter[i]);
    if (response != ARCHIVE_OK) {
      Rf_error(archive_error_string(a));
    }
  }

  archive_write_open_filename(a, archive_filename.c_str());
  for (int i = 0; i < files.size(); ++i) {
    stat(files[i], &st);
    entry = archive_entry_new();
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, files[i]);
    archive_write_header(a, entry);
    if ((fd = open(files[i], O_RDONLY)) != -1) {
      len = read(fd, buff, sizeof(buff));
      while (len > 0) {
        archive_write_data(a, buff, len);
        len = read(fd, buff, sizeof(buff));
      }
      close(fd);
    }
    archive_entry_free(entry);
  }
  archive_write_free(a);

  return R_NilValue;
#endif
}
