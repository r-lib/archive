#include "connection.h"

SEXP new_connection(
    const char* description,
    const char* mode,
    const char* class_name,
    Rconnection* ptr) {
  return R_new_custom_connection(description, mode, class_name, ptr);
}
