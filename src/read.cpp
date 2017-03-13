#include "r_archive.h"
#include <Rcpp.h>

/* Read archives
 *
 * The custom R connection code was adapted from curl package by Jeroen Ooms
 * Source: https://github.com/cran/curl/blob/3897ba5203dee940e2ce40ac23a0d1106da93df6/src/curl.c
 *
 * The libarchive code from libarchive examples
 * Source: https://github.com/libarchive/libarchive/wiki/Examples
 */

static Rboolean rchive_read_open(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;

  if ((r->last_response = archive_read_open_filename(r->ar, r->archive_filename, 10240)) != ARCHIVE_OK) {
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

void rchive_read_close(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;
  archive_read_close(r->ar);

  con->isopen = FALSE;
}

void rchive_read_destroy(Rconnection con) {
  rchive *r = (rchive *) con->private_ptr;

  /* free the handle connection */
  archive_read_free(r->ar);
  free(r->buf);
  free(r->archive_filename);
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
SEXP read_connection(const std::string & archive_filename, const std::string & filename, size_t sz = 16384) {
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection("input", "rb", "archive", &con));

  /* Setup archive */
  rchive *r = (rchive *) malloc(sizeof(rchive));
  r->limit = sz;
  r->buf = (char *) malloc(r->limit);

  r->archive_filename = (char *) malloc(strlen(archive_filename.c_str()) + 1);
  strcpy(r->archive_filename, archive_filename.c_str());

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
  con->open = rchive_read_open;
  con->close = rchive_read_close;
  con->destroy = rchive_read_destroy;
  con->read = rchive_read;

  UNPROTECT(1);
  return rc;
}

