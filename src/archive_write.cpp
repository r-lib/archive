#include "r_archive.h"
#include <fcntl.h>
#include <string.h>
#include <vector>

std::string my_basename(std::string const& str) {
  std::size_t found = str.find_last_of("/\\");
  return str.substr(found + 1);
}

/* callback function to store received data */
static size_t
rchive_write_data(const void* contents, size_t sz, size_t n, Rconnection ctx) {
  rchive* r = (rchive*)ctx->private_ptr;

  size_t realsize = sz * n;
  call(archive_write_data, ctx, contents, realsize);
  r->size += realsize;

  return n;
}

std::string scratch_file(const char* filename) {
  static auto tempdir = cpp11::package("base")["tempdir"];
  std::string out =
      std::string(CHAR(STRING_ELT(tempdir(), 0))) + '/' + my_basename(filename);
  return out;
}

static Rboolean rchive_write_open(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  r->ar = archive_write_disk_new();

  r->entry = archive_entry_new();

  archive_entry_set_pathname(
      r->entry, scratch_file(r->filename.c_str()).c_str());
  archive_entry_set_filetype(r->entry, AE_IFREG);
  archive_entry_set_perm(r->entry, 0644);
  call(archive_write_header, con, r->entry);

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

  if (!con->isopen) {
    return;
  }
  /* Close scratch file */
  call(archive_write_finish_entry, con);
  call(archive_write_close, con);
  call(archive_write_free, con);
  archive_entry_free(r->entry);
  con->isopen = FALSE;

  /* Write scratch file to archive */
  struct archive* in;
  struct archive* out;
  struct archive_entry* entry;
  in = archive_read_disk_new();
#ifndef __MINGW32__
  call(archive_read_disk_set_standard_lookup, in);
#endif
  entry = archive_entry_new();

  std::string scratch = scratch_file(r->filename.c_str());
  int fd = open(scratch.c_str(), O_RDONLY);
  if (fd < 0) {
    Rf_error("Could not open scratch file");
  }
  archive_entry_copy_pathname(entry, r->filename.c_str());
  call(
      archive_read_disk_entry_from_file,
      in,
      entry,
      fd,
      (const struct stat*)NULL);

  out = archive_write_new();

  call(archive_write_set_format, out, r->format);

  for (int i = 0; i < FILTER_MAX && r->filters[i] != -1; ++i) {
    call(archive_write_add_filter, out, r->filters[i]);
  }

  if (!r->options.empty()) {
    call(archive_write_set_options, out, r->options.c_str());
  }

  call(archive_write_open_filename, out, r->archive_filename.c_str());
  call(archive_write_header, out, entry);

  while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
    call(archive_write_data, out, buf, bytes_read);
  }
  close(fd);
  archive_entry_free(entry);
  call(archive_write_free, out);
  call(archive_read_free, in);

  unlink(scratch.c_str());
}

void rchive_write_destroy(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  /* free the handle connection */
  delete r;
}

// This writes a single file to a new connection, it first writes the data
// to a scratch file, then adds it to the archive, because the archive
// headers need to be written before the data is added, and we do not know
// the size of the data until it has been written.
[[cpp11::register]] SEXP archive_write_(
    const std::string& archive_filename,
    const std::string& filename,
    const std::string& mode,
    int format,
    cpp11::integers filters,
    cpp11::strings options,
    size_t sz) {
  Rconnection con;
  SEXP rc =
      PROTECT(new_connection("input", mode.c_str(), "archive_write", &con));

  /* Setup archive */
  rchive* r = (rchive*)new rchive;

  r->archive_filename = std::move(archive_filename);

  r->format = format;

  // Initialize filters
  if (filters.size() > FILTER_MAX) {
    cpp11::stop("Cannot use more than %i filters", FILTER_MAX);
  }
  for (int i = 0; i < FILTER_MAX; ++i) {
    r->filters[i] = -1;
  }
  for (int i = 0; i < filters.size(); ++i) {
    r->filters[i] = filters[i];
  }

  r->filename = std::move(filename);

  if (options.size() > 0) {
    r->options = options[0];
  }

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
