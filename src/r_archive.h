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
};

[[cpp11::register]] void rchive_init(SEXP xptr);
