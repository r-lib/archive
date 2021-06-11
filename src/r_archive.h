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

#include <vector>

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define R_EOF -1

#define FILTER_MAX 8

struct rchive {
  std::string archive_filename;
  int format;
  std::string filename;
  std::vector<char> buf;
  char* cur;
  archive* ar;
  archive_entry* entry;
  int last_response;
  int has_more;
  size_t size;
  int filters[FILTER_MAX];
  std::string options;
};

size_t pop(void* target, size_t max, rchive* r);

size_t push(rchive* r);

#if ARCHIVE_VERSION_NUMBER < 3000004
int archive_write_add_filter(struct archive* a, int code);
#endif

[[cpp11::register]] void rchive_init(SEXP xptr);
