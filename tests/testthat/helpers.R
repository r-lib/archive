
# Write a valid ZIP whose single entry's local header lives past `lh_off` bytes,
# using a sparse hole so the file is only a few KB on disk despite its apparent
# size. Reading the entry forces libarchive to seek() to an absolute offset
# beyond 2^31, which is the case that regressed in #81 (int64 offset truncated
# to a 32-bit int on the way to base::seek()).
write_sparse_zip <- function(path, lh_off = 2200000000, content = "hello") {
  nb <- charToRaw("a")
  nlen <- length(nb)
  data <- charToRaw(content)
  dlen <- length(data)
  u16 <- function(x) as.raw(c(bitwAnd(x, 255L), bitwAnd(bitwShiftR(x, 8), 255L)))
  u32 <- function(x) {
    x <- as.numeric(x)
    as.raw(c(x %% 256, (x %/% 256) %% 256, (x %/% 65536) %% 256, (x %/% 16777216) %% 256))
  }

  # local file header (stored, no data descriptor), CRC left 0 (not verified
  # when only reading headers)
  lfh <- c(
    as.raw(c(0x50, 0x4b, 0x03, 0x04)), u16(20), u16(0), u16(0), u16(0), u16(0),
    u32(0), u32(dlen), u32(dlen), u16(nlen), u16(0), nb, data
  )
  cd_off <- lh_off + length(lfh)
  cdh <- c(
    as.raw(c(0x50, 0x4b, 0x01, 0x02)), u16(20), u16(20), u16(0), u16(0), u16(0),
    u16(0), u32(0), u32(dlen), u32(dlen), u16(nlen), u16(0), u16(0), u16(0),
    u16(0), u32(0), u32(lh_off), nb
  )
  eocd <- c(
    as.raw(c(0x50, 0x4b, 0x05, 0x06)), u16(0), u16(0), u16(1), u16(1),
    u32(length(cdh)), u32(cd_off), u16(0)
  )

  con <- file(path, "wb")
  on.exit(close(con))
  seek(con, where = lh_off) # leaves a sparse hole before the local header
  writeBin(c(lfh, cdh, eocd), con)
  invisible(path)
}

skip_if_no_encoding <- function(enc) {
  tryCatch({
    iconv("xxx", from = env, to = env)
    TRUE
  }, error = function(err) NULL)
  testthat::skip(paste("No encoding:", enc))
}
