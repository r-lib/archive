#include "r_archive.h"

SEXP new_connection_xptr;

SEXP read_connection_xptr;

void rchive_init(SEXP nc_xptr, SEXP rc_xptr) {
  new_connection_xptr = nc_xptr;
  R_PreserveObject(nc_xptr);
  read_connection_xptr = rc_xptr;
  R_PreserveObject(rc_xptr);
}

SEXP new_connection(
    const char* description,
    const char* mode,
    const char* class_name,
    Rconnection* ptr) {
  auto new_connection_ptr = reinterpret_cast<SEXP (*)(
      const char*, const char*, const char*, Rconnection*)>(
      R_ExternalPtrAddr(new_connection_xptr));
  return new_connection_ptr(description, mode, class_name, ptr);
}

size_t read_connection(SEXP connection, void* buf, size_t n) {
  auto read_connection_ptr = reinterpret_cast<size_t (*)(SEXP, void*, size_t)>(
      R_ExternalPtrAddr(read_connection_xptr));
  return read_connection_ptr(connection, buf, n);
}

size_t pop(void* target, size_t max, rchive* r) {
  size_t copy_size = r->size < max ? r->size : max;
  memcpy(target, r->cur, copy_size);
  r->cur += copy_size;
  r->size -= copy_size;

  /* clang-format off */
  /* Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", max, copy_size, r->size); */
  /* clang-format on */

  return copy_size;
}

size_t push(rchive* r) {
  R_CheckUserInterrupt();
  const void* buf;
  size_t size;
  __LA_INT64_T offset;

  if (r->last_response == ARCHIVE_EOF) {
    return 0;
  }

  if (!r->cur) {
    r->cur = r->buf.data();
  }

  /* move existing data to front of buffer (if any) */
  memmove(r->buf.data(), r->cur, r->size);

  /* read data from archive */
  r->last_response = archive_read_data_block(r->ar, &buf, &size, &offset);
  if (r->last_response == ARCHIVE_EOF) {
    r->has_more = 0;
    return 0;
  }
  if (r->last_response != ARCHIVE_OK) {
    Rf_error(archive_error_string(r->ar));
  }

  /* allocate more space if required */
  size_t newsize = r->size + size;
  while (newsize > r->buf.size()) {
    size_t newlimit = 2 * r->buf.size();
    // Rprintf("Resizing buffer to %d.\n", newlimit);
    r->buf.resize(newlimit);
  }

  /* append new data */
  /* Rprintf("Pushed %d bytes, new size %d bytes.\n", size, newsize); */
  memcpy(r->buf.data() + r->size, buf, size);
  r->size = newsize;
  r->cur = r->buf.data();
  return size;
}

#if ARCHIVE_VERSION_NUMBER < 3000004
/* Define archive_write_add_filter for older versions */
/* This code is pulled directly from
 * https://github.com/libarchive/libarchive/blob/06052e47e500ef4c8c937c4c8b987433a647cb4c/libarchive/archive_write_add_filter.c
 *-
 * Copyright (c) 2012 Ondrej Holy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cerrno>

/* A table that maps filter codes to functions. */
static struct {
  int code;
  int (*setter)(struct archive*);
} codes[] = {
    {ARCHIVE_FILTER_NONE, archive_write_add_filter_none},
    {ARCHIVE_FILTER_GZIP, archive_write_add_filter_gzip},
    {ARCHIVE_FILTER_BZIP2, archive_write_add_filter_bzip2},
    {ARCHIVE_FILTER_COMPRESS, archive_write_add_filter_compress},
    {ARCHIVE_FILTER_LZMA, archive_write_add_filter_lzma},
    {ARCHIVE_FILTER_XZ, archive_write_add_filter_xz},
    {ARCHIVE_FILTER_LZIP, archive_write_add_filter_lzip},
    {-1, NULL}};

int archive_write_add_filter(struct archive* a, int code) {
  int i;

  for (i = 0; codes[i].code != -1; i++) {
    if (code == codes[i].code)
      return ((codes[i].setter)(a));
  }

  archive_set_error(a, EINVAL, "No such filter");
  return (ARCHIVE_FATAL);
}
#endif
