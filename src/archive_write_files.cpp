#include "r_archive.h"
#include <fcntl.h>

// Write files already on disk to a new archive
[[cpp11::register]] SEXP archive_write_files_(
    const std::string& archive_filename,
    cpp11::strings files,
    int format,
    cpp11::integers filters,
    cpp11::strings options,
    size_t sz = 16384) {

  struct archive* a;
  struct archive_entry* entry;
  struct stat st;
  std::vector<char> buf;
  int len;
  int fd;

  buf.resize(sz);

  a = archive_write_new();

  call(archive_write_set_format, a, format);

  for (int i = 0; i < filters.size(); ++i) {
    call(archive_write_add_filter, a, filters[i]);
  }

  if (options.size() > 0) {
    call(archive_write_set_options, a, std::string(options[0]).c_str());
  }

  call(archive_write_open_filename, a, archive_filename.c_str());
  for (std::string file : files) {
    stat(file.c_str(), &st);
    entry = archive_entry_new();
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, file.c_str());
    call(archive_write_header, a, entry);
    if ((fd = open(file.c_str(), O_RDONLY)) != -1) {
      len = read(fd, buf.data(), buf.size());
      while (len > 0) {
        call(archive_write_data, a, buf.data(), len);
        len = read(fd, buf.data(), buf.size());
      }
      close(fd);
    }
    archive_entry_free(entry);
  }
  call(archive_write_free, a);

  return R_NilValue;
}
