#pragma once

#include "Rinternals.h"

#define class class_name
#define private private_ptr
#include <R_ext/Connections.h>
#undef class
#undef private

#ifdef __cplusplus
extern "C" {
#endif

SEXP new_connection(
    const char* description,
    const char* mode,
    const char* class_name,
    Rconnection* ptr);

#ifdef __cplusplus
}
#endif
