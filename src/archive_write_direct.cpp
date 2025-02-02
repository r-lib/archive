#include "r_archive.h"
#include <fcntl.h>
#include <string.h>
#include <vector>

/* callback function to store received data */
static size_t rchive_write_direct_data(
    const void* contents, size_t sz, size_t n, Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  size_t realsize = sz * n;
  call(archive_write_data, con, contents, realsize);
  r->size += realsize;

  return n;
}

static Rboolean rchive_write_direct_open(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  local_utf8_locale ll;

  r->ar = archive_write_new();

  for (int i = 0; i < FILTER_MAX && r->filters[i] != -1; ++i) {
    call(archive_write_add_filter, con, r->filters[i]);
  }

  call(archive_write_set_format, con, r->format);

  if (!cpp11::is_na(r->password[0])) {
    call(archive_write_set_passphrase, con, std::string(r->password[0]).c_str());
  }

  if (!r->options.empty()) {
    call(archive_write_set_options, con, r->options.c_str());
  }

  call(archive_write_open_filename, con, r->archive_filename.c_str());

  r->entry = archive_entry_new();

  archive_entry_set_pathname(r->entry, r->filename.c_str());
  archive_entry_set_filetype(r->entry, AE_IFREG);
  archive_entry_set_perm(r->entry, 0644);
  archive_entry_unset_size(r->entry);
  call(archive_write_header, con, r->entry);

  archive_entry_free(r->entry);

  con->isopen = TRUE;

  return TRUE;
}

/* This function closes the temporary scratch file, then writes the actual
 * archive file based on the archive filename given and then unlinks the
 * scratch file */
void rchive_write_direct_close(Rconnection con) {
  if (!con->isopen) {
    return;
  }
  /* Close scratch file */
  call(archive_write_close, con);
  call(archive_write_free, con);

  con->isopen = FALSE;
}

void rchive_write_direct_destroy(Rconnection con) {
  rchive* r = (rchive*)con->private_ptr;

  /* free the handle connection */
  delete r;
}

// This writes a single (direct) file to a new connection. Unlike other archive
// formats direct does not need to know the size of the file up front.
// This lets us write to it without having to write to a scratch file first.
[[cpp11::register]] SEXP archive_write_direct_(
    const std::string& archive_filename,
    const std::string& filename,
    std::string mode,
    int format,
    cpp11::integers filters,
    cpp11::strings options,
    cpp11::strings password,
    size_t sz) {
  Rconnection con;
  SEXP rc =
      PROTECT(new_connection("input", mode.c_str(), "archive_write", &con));

  /* Setup archive */
  rchive* r = (rchive*)new rchive;

  r->archive_filename = std::move(archive_filename);

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

  r->format = format;
  r->password = password;

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
  con->open = rchive_write_direct_open;
  con->close = rchive_write_direct_close;
  con->destroy = rchive_write_direct_destroy;
  con->write = rchive_write_direct_data;

  UNPROTECT(1);
  return rc;
}
