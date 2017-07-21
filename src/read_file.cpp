#include "r_archive.h"
#include <Rcpp.h>
#include <fcntl.h>

static Rboolean file_read_open(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  con->text = strchr(con->mode, 'b') ? FALSE : TRUE;

  r->ar = archive_read_new();
  archive_read_support_filter_all(r->ar);
  archive_read_support_format_raw(r->ar);

  r->last_response = archive_read_open_filename(r->ar, r->filename, r->limit);
  if (r->last_response != ARCHIVE_OK) {
    Rf_error(archive_error_string(r->ar));
  }
  r->last_response = archive_read_next_header(r->ar, &r->entry);
  if (r->last_response != ARCHIVE_OK) {
    Rf_error(archive_error_string(r->ar));
  }

  r->size = archive_entry_size(r->entry);
  r->cur = r->buf;
  r->has_more = 1;
  copy_data(r);

  con->isopen = TRUE;
  return TRUE;
}

static size_t
file_read_data(void* target, size_t sz, size_t ni, Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;
  size_t size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, size, r);
  while ((size > total_size) && r->has_more) {
    copy_data(r);
    total_size += pop((char*)target + total_size, (size - total_size), r);
  }
  con->incomplete = (Rboolean)r->has_more;
  return total_size;
}

/* This function closes the temporary scratch file, then writes the actual
 * archive file based on the archive filename given and then unlinks the
 * scratch archive */
void file_read_close(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  archive_read_free(r->ar);
  con->isopen = FALSE;
}

void file_read_destroy(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  /* free the handle connection */
  free(r->filename);
  free(r);
}

/* naive implementation of readLines */
static int file_read_getc(Rconnection con) {
  int x = 0;
#ifdef WORDS_BIGENDIAN
  return file_read_data(&x, 1, 1, con) ? BSWAP_32(x) : R_EOF;
#else
  return file_read_data(&x, 1, 1, con) ? x : R_EOF;
#endif
}

// Get a connection to a single non-archive file, optionally with one or more
// filters.
// [[Rcpp::export]]
SEXP read_file_connection(
    const std::string& filename, const std::string& mode, size_t sz = 16384) {
  Rconnection con;
  SEXP rc = PROTECT(
      R_new_custom_connection("file_input", mode.c_str(), "archive", &con));

  /* Setup archive */
  rchive* r = (rchive*)malloc(sizeof(rchive));

  r->limit = sz;
  r->buf = (char*)malloc(r->limit);

  r->filename = (char*)malloc(strlen(filename.c_str()) + 1);
  strcpy(r->filename, filename.c_str());

  /* set connection properties */
  con->incomplete = TRUE;
  con->private_ptr = r;
  con->canread = TRUE;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->open = file_read_open;
  con->close = file_read_close;
  con->destroy = file_read_destroy;
  con->read = file_read_data;
  con->fgetc = file_read_getc;
  con->fgetc_internal = file_read_getc;

  UNPROTECT(1);
  return rc;
}
