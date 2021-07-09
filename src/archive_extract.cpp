#include "r_archive.h"

static int copy_data(struct archive* ar, struct archive* aw) {
  int r;
  const void* buff;
  size_t size;
  int64_t offset;

  for (;;) {
    r = call(archive_read_data_block, ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return (ARCHIVE_OK);
    }
    call(archive_write_data_block, aw, buff, size, offset);
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
    cpp11::strings options,
    size_t sz = 16384) {
  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;
  int flags;
  int r;

  local_utf8_locale ll;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  call(archive_read_support_format_all, a);
  call(archive_read_support_filter_all, a);

  if (options.size() > 0) {
    call(archive_read_set_options, a, std::string(options[0]).c_str());
  }

  ext = archive_write_disk_new();
  call(archive_write_disk_set_options, ext, flags);
#ifndef __MINGW32__
  /* set_standard_lookup is not available on windows */
  call(archive_write_disk_set_standard_lookup, ext);
#endif
  call(archive_read_open_filename, a, archive_filename.c_str(), sz);
  for (;;) {
    r = call(archive_read_next_header, a, &entry);
    if (r == ARCHIVE_EOF)
      break;
    const char* filename = archive_entry_pathname(entry);
    if (filenames.size() == 0 || any_matches(filename, filenames)) {
      call(archive_write_header, ext, entry);
      copy_data(a, ext);
      call(archive_write_finish_entry, ext);
    }
  }
  call(archive_read_close, a);
  call(archive_read_free, a);
  call(archive_write_close, ext);
  call(archive_write_free, ext);
}
