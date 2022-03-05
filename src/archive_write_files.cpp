#include "r_archive.h"
#include <cli/progress.h>
#include <fcntl.h>

const char* const pb_format =
    "{cli::pb_spin} %zu added | {cli::pb_current_bytes} "
    "({cli::pb_rate_bytes}) | "
    "{cli::pb_elapsed}";

// Write files already on disk to a new archive
[[cpp11::register]] SEXP archive_write_files_(
    const std::string& archive_filename,
    cpp11::strings files,
    int format,
    cpp11::integers filters,
    cpp11::strings options,
    const std::string& password,
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

  if (password.size() > 0) {
    call(archive_write_set_passphrase, a, password.c_str());
  }

  size_t num_written = 0;
  size_t total_written = 0;

  using namespace cpp11::literals;

  cpp11::sexp progress_bar(cli_progress_bar(NA_INTEGER, R_NilValue));

  call(archive_write_open_filename, a, archive_filename.c_str());
  for (std::string file : files) {
    stat(file.c_str(), &st);
    entry = archive_entry_new();
#if defined(_WIN32) || (!defined(__GNUC__) && !defined(__clang__))
    // there are quite many CRT dialects and passing struct stat to 3rdparty library could be unstable.
    archive_entry_set_size(entry, st.st_size);
    archive_entry_set_mtime(entry, st.st_mtime, 0);
    archive_entry_set_ctime(entry, st.st_ctime, 0);
    archive_entry_set_atime(entry, st.st_atime, 0);
    archive_entry_set_mode(entry, st.st_mode); // seems required as not defaulting to S_IFREG.
#else
    archive_entry_copy_stat(entry, &st);
#endif
    archive_entry_set_pathname(entry, file.c_str());
    call(archive_write_header, a, entry);
    if ((fd = open(file.c_str(), O_RDONLY)) != -1) {
      len = read(fd, buf.data(), buf.size());
      while (len > 0) {
        call(archive_write_data, a, buf.data(), len);
        total_written += len;
        if (CLI_SHOULD_TICK) {
          cli_progress_set_format(progress_bar, pb_format, num_written);

          cli_progress_set(progress_bar, total_written);
        }
        len = read(fd, buf.data(), buf.size());
      }
      close(fd);
    }
    archive_entry_free(entry);
    ++num_written;
  }
  call(archive_write_free, a);

  cli_progress_done(progress_bar);

  return R_NilValue;
}
