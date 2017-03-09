#include <archive.h>
#include <archive_entry.h>
#include "Rcpp.h"

#define class class_name
#define private private_ptr
#include <R_ext/Connections.h>
#undef class
#undef private

#if R_CONNECTIONS_VERSION != 1
#error "Missing or unsupported connection API in R"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(R_VERSION) && R_VERSION >= R_Version(3, 3, 0)
static Rconnection get_connection(SEXP con) {
  return R_GetConnection(con);
}
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
  char* archive_filename;
  char* filename;
  char* buf;
  char* cur;
  archive *ar;
  archive_entry *entry;
  int last_response;
  int has_more;
  size_t limit;
  size_t size;
} rchive;

#ifdef __cplusplus
}
#endif
