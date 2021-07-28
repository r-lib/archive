#include "connection.h"

SEXP new_connection(
    const char* description,
    const char* mode,
    const char* class_name,
    Rconnection* ptr) {
  return R_new_custom_connection(description, mode, class_name, ptr);
}

size_t read_connection(SEXP connection, void* buf, size_t n) {
  return R_ReadConnection(R_GetConnection(connection), buf, n);
}
