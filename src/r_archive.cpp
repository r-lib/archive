#include "r_archive.h"

size_t pop(void *target, size_t max, rchive *r) {
  size_t copy_size = min(r->size, max);
  memcpy(target, r->cur, copy_size);
  r->cur += copy_size;
  r->size -= copy_size;
  return copy_size;
}

void copy_data(rchive *r) {
  R_CheckUserInterrupt();
  __LA_INT64_T offset;
  if(r->last_response != ARCHIVE_EOF) {

    /* move existing data to front of buffer (if any) */
    memcpy(r->buf, r->cur, r->size);

    r->last_response = archive_read_data_block(r->ar, (const void **) &r->buf, &r->size, &offset);
    if (r->last_response == ARCHIVE_EOF) {
      r->has_more = 0;
      return;
    }
    if (r->last_response != ARCHIVE_OK) {
      Rcpp::stop(archive_error_string(r->ar));
    }
    r->cur = r->buf;
  }
}
