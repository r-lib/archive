#pragma once

#include "Rinternals.h"

// clang-format off
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define class class_name
#define private private_ptr
#include <R_ext/Connections.h>
#undef class
#undef private
#ifdef __clang__
# pragma clang diagnostic pop
#endif
// clang-format on

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
