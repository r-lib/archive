#include "r_archive.h"
#include <Rcpp.h>
#include <fcntl.h>
#include <string.h>

std::string my_basename(std::string const& str) {
  std::size_t found = str.find_last_of("/\\");
  return str.substr(found + 1);
}

/* callback function to store received data */
static size_t
rchive_write_data(const void* contents, size_t sz, size_t n, Rconnection ctx) {
  rchive* r = (rchive*)ctx->private_ptr;

  size_t realsize = sz * n;
  archive_write_data(r->ar, contents, realsize);
  r->size += realsize;

  return n;
}

std::string scratch_file(const char* filename) {
  static Rcpp::Function tempdir("tempdir", R_BaseEnv);
  std::string out =
      std::string(CHAR(STRING_ELT(tempdir(), 0))) + '/' + my_basename(filename);
  return out;
}

static Rboolean rchive_write_open(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  r->ar = archive_write_disk_new();

  r->entry = archive_entry_new();

  archive_entry_set_pathname(r->entry, scratch_file(r->filename).c_str());
  archive_entry_set_filetype(r->entry, AE_IFREG);
  archive_entry_set_perm(r->entry, 0644);
  archive_write_header(r->ar, r->entry);

  con->isopen = TRUE;
  return TRUE;
}

/* This function closes the temporary scratch file, then writes the actual
 * archive file based on the archive filename given and then unlinks the
 * scratch file */
void rchive_write_close(Rconnection con) {
  char buf[8192];
  size_t bytes_read;
  rchive* r = (rchive*)con->private_ptr;
  int response;

  /* Close scratch file */
  archive_write_finish_entry(r->ar);
  archive_write_close(r->ar);
  archive_write_free(r->ar);
  archive_entry_free(r->entry);

  /* Write scratch file to archive */
  struct archive* in;
  struct archive* out;
  struct archive_entry* entry;
  in = archive_read_disk_new();
  archive_read_disk_set_standard_lookup(in);
  entry = archive_entry_new();

  std::string scratch = scratch_file(r->filename);
  int fd = open(scratch.c_str(), O_RDONLY);
  if (fd < 0) {
    Rf_error("Could not open scratch file");
  }
  archive_entry_copy_pathname(entry, r->filename);
  response = archive_read_disk_entry_from_file(in, entry, fd, NULL);
  if (response != ARCHIVE_OK) {
    Rf_error(archive_error_string(in));
  }

  out = archive_write_new();

  response = archive_write_set_format(out, r->format);
  if (response != ARCHIVE_OK) {
    Rf_error(archive_error_string(out));
  }

  for (size_t i = 0; i < 1; ++i) {
    response = archive_write_add_filter(out, r->filter);
    if (response != ARCHIVE_OK) {
      Rf_error(archive_error_string(out));
    }
  }

  response = archive_write_open_filename(out, r->archive_filename);
  if (response != ARCHIVE_OK) {
    Rf_error(archive_error_string(out));
  }
  response = archive_write_header(out, entry);
  if (response != ARCHIVE_OK) {
    Rf_error(archive_error_string(out));
  }

  while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
    int bytes_out = archive_write_data(out, buf, bytes_read);
    if (bytes_out < 0) {
      Rf_error("Error writing to '%s'", r->archive_filename);
    }
  }
  close(fd);
  archive_entry_free(entry);
  archive_write_free(out);
  archive_read_free(in);

  unlink(scratch.c_str());
  con->isopen = FALSE;
}

void rchive_write_destroy(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  /* free the handle connection */
  free(r->archive_filename);
  free(r->filename);
  free(r);
}

// This writes a single file to a new connection, it first writes the data to a
// scratch file, then adds it to the archive, because the archive headers need
// to be written before the data is added, and we do not know the size of the
// data until it has been written.
// [[Rcpp::export]]
SEXP write_connection(
    const std::string& archive_filename,
    const std::string& filename,
    int format,
    int filter,
    size_t sz = 16384) {
  Rconnection con;
  SEXP rc =
      PROTECT(R_new_custom_connection("input", "wb", "archive_write", &con));

  /* Setup archive */
  rchive* r = (rchive*)malloc(sizeof(rchive));
  // r->limit = sz;
  // r->buf = (char *) malloc(r->limit);
  // r->cur = r->buf;
  // r->size = 0;

  r->archive_filename = (char*)malloc(strlen(archive_filename.c_str()) + 1);
  strcpy(r->archive_filename, archive_filename.c_str());

  r->format = format;
  r->filter = filter;

  r->filename = (char*)malloc(strlen(filename.c_str()) + 1);
  strcpy(r->filename, filename.c_str());

  /* set connection properties */
  con->incomplete = TRUE;
  con->private_ptr = r;
  con->canread = FALSE;
  con->canseek = FALSE;
  con->canwrite = TRUE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = FALSE;
  con->open = rchive_write_open;
  con->close = rchive_write_close;
  con->destroy = rchive_write_destroy;
  con->write = rchive_write_data;

  UNPROTECT(1);
  return rc;
}
