#include <cpp11.hpp>

#include "connection/connection.h"

#undef Realloc
// Also need to undefine the Free macro
#undef Free

#include <archive.h>
#include <archive_entry.h>

#undef TRUE
#undef FALSE
#include <R_ext/Boolean.h>

#include <clocale>
#include <vector>

#define R_EOF -1

#define FILTER_MAX 8

struct rchive {
  std::string archive_filename;
  int format;
  std::string filename;
  cpp11::sexp file;
  int file_offset;
  cpp11::sexp con;
  std::vector<char> buf;
  char* cur;
  archive* ar = nullptr;
  archive_entry* entry = nullptr;
  ssize_t last_response = 0;
  bool has_more = true;
  size_t size = 0;
  int filters[FILTER_MAX];
  std::string options;
};

size_t pop(void* target, size_t max, rchive* r);

size_t push(rchive* r);

ssize_t myread(struct archive* a, void* client_data, const void** buff);
int64_t myseek(struct archive*, void* client_data, int64_t offset, int whence);
int myclose(struct archive* a, void* client_data);

#if ARCHIVE_VERSION_NUMBER < 3000004
int archive_write_add_filter(struct archive* a, int code);
#endif

#define call(f, ...) call_(__FILE__, __LINE__, #f, f, __VA_ARGS__)

template <typename F, typename... Args>
inline int call_(
    const char* file_name,
    int line,
    const char* function_name,
    F f,
    Rconnection con,
    Args... args) {
  rchive* r = (rchive*)con->private_ptr;
  if (!r->ar) {
    return ARCHIVE_OK;
  }
  r->last_response = f(r->ar, args...);
  if (r->last_response < ARCHIVE_OK) {
    con->isopen = FALSE;
    const char* msg = archive_error_string(r->ar);
    if (msg) {
      Rf_errorcall(
          R_NilValue, "%s:%i %s(): %s", file_name, line, function_name, msg);
    } else {
      Rf_errorcall(
          R_NilValue,
          "%s:%i %s(): unknown libarchive error",
          file_name,
          line,
          function_name);
    }
  }
  return r->last_response;
}

template <typename F, typename... Args>
inline int call_(
    const char* file_name,
    int line,
    const char* function_name,
    F f,
    archive* ar,
    Args... args) {
  ssize_t response = f(ar, args...);
  if (response < ARCHIVE_OK) {
    const char* msg = archive_error_string(ar);
    if (msg) {
      Rf_errorcall(
          R_NilValue, "%s:%i %s(): %s", file_name, line, function_name, msg);
    } else {
      Rf_errorcall(
          R_NilValue,
          "%s:%i %s(): unknown libarchive error",
          file_name,
          line,
          function_name);
    }
  }
  return response;
}

class local_utf8_locale {
  // In the future once R is using the windows runtime that supports UTF-8 we
  // could set the UTF-8 locale here for windows as well with ".UTF-8"
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-160#utf-8-support
  // But for now just do nothing
#ifdef __MINGW32__
#else
private:
  std::string old_locale_;

public:
  local_utf8_locale() : old_locale_(std::setlocale(LC_CTYPE, NULL)) {
#ifdef __APPLE__
    const char* locale = "UTF-8";
#else
    const char* locale = "C.UTF-8";
#endif
    const char* new_locale = std::setlocale(LC_CTYPE, locale);
    if (nullptr == new_locale) {
      cpp11::warning("Setting UTF-8 locale failed");
    }
  }
  ~local_utf8_locale() { std::setlocale(LC_CTYPE, old_locale_.c_str()); }
#endif
};

class local_connection {
private:
  cpp11::sexp connection_;
  std::string mode_;
  bool opened_;

  cpp11::function close = cpp11::package("base")["close"];

public:
  local_connection(
      const cpp11::sexp& connection, const std::string& mode = "rb")
      : connection_(connection), mode_(mode), opened_(false) {
    static auto isOpen = cpp11::package("base")["isOpen"];
    opened_ = !isOpen(connection);
    if (opened_) {
      static auto open = cpp11::package("base")["open"];
      open(connection_, mode.c_str());
    }
  }
  ~local_connection() {
    if (opened_) {
      close(connection_);
    }
  }
  operator SEXP() const { return connection_; }
  operator cpp11::sexp() const { return connection_; }
};

[[cpp11::register]] void rchive_init(SEXP xptr);
