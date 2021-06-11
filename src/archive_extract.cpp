#include "r_archive.h"

static int copy_data(struct archive* ar, struct archive* aw) {
  int r;
  const void* buff;
  size_t size;
  int64_t offset;

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return (ARCHIVE_OK);
    }
    if (r != ARCHIVE_OK) {
      cpp11::stop("archive_read_data_block(): %s", archive_error_string(ar));
    }
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK) {
      cpp11::stop("archive_write_data_block(): %s", archive_error_string(aw));
    }
  }
}

bool any_matches(const char* filename, cpp11::strings filenames) {
  for (std::string f : filenames) {
    if (strcmp(filename, f.c_str()) == 0) {
      return true;
    }
  }
  return false;
}

[[cpp11::register]] void archive_extract_(
    const std::string& archive_filename,
    cpp11::strings filenames,
    size_t sz = 16384) {
  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;
  int flags;
  int r;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);
  if ((r = archive_read_open_filename(a, archive_filename.c_str(), sz))) {
    cpp11::stop("Could not open '%s'", archive_filename.c_str());
  }
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      break;
    if (r != ARCHIVE_OK) {
      cpp11::stop("archive_read_next_header(): %s", archive_error_string(a));
    }
    const char* filename = archive_entry_pathname(entry);
    if (filenames.size() == 0 || any_matches(filename, filenames)) {
      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK) {
        cpp11::stop("archive_write_header(): %s", archive_error_string(ext));
      } else {
        copy_data(a, ext);
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
          cpp11::stop(
              "archive_write_finish_entry(): %s", archive_error_string(ext));
        }
      }
    }
  }
  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
}
