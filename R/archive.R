## usethis namespace: start
#' @useDynLib archive, .registration = TRUE
#' @importFrom tibble as_tibble
#' @importFrom cli cli_progress_bar
## usethis namespace: end
NULL

#' Construct a new archive
#'
#' This function retrieves metadata about files in an archive, it can be passed
#' to [archive_read()] or [archive_write] to create a connection to read or
#' write a specific file from the archive.
#'
#' @param file File path to the archive.
#' @inheritParams archive_read
#' @seealso [archive_read()], [archive_write()] to read and write archive files
#' using R connections, [archive_extract()], [archive_write_files()],
#' [archive_write_dir()] to add or extract files from an archive.
#' @return A [tibble][tibble::tibble-package] with details about files in the archive.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' a
#' @export
archive <- function(file, options = character()) {
  if (!inherits(file, "connection")) {
    file <- file(file, "rb")
  }

  if (!isOpen(file)) {
    open(file, "rb")
  }

  options <- validate_options(options)

  res <- archive_(file, options)

  res
}

filter_by_extension <- function(file) {

  extension_to_filter <- function(ext) {
    switch(ext,
      Z = "compress",

      # There is currently no base64enc constant in libarchive
      # https://github.com/libarchive/libarchive/pull/907
      # base64enc = "base64enc"

      bz2 = "bzip2",
      gz = "gzip",
      lz = "lzip",
      lz4 = "lz4",
      lzo = "lzop",
      lzma = "lzma",
      uu = "uuencode",
      xz = "xz",
      zst = "zstd",

      NULL)
  }

  extensions <- sub("^[^.][.]", "", basename(file))

  Reduce(`c`, Map(extension_to_filter, strsplit(extensions, "[.]")[[1]]))
}

format_and_filter_by_extension <- function(file) {
  ext <- sub("^[^.]*[.]", "", basename(file))
  switch(ext,
    "7z" = list("7zip", "none"),

    "cpio" = list("cpio", "none"),

    "iso" = list("iso9660", "none"),

    "mtree" = list("mtree", "none"),

    "tar" = list("tar", "none"),

    "tgz" = list("tar", "gzip"),
    "taz" = list("tar", "gzip"),
    "tar.gz" = list("tar", "gzip"),

    "tbz" = list("tar", "bzip2"),
    "tbz2" = list("tar", "bzip2"),
    "tz2" = list("tar", "bzip2"),
    "tar.bz2" = list("tar", "bzip2"),

    "tlz" = list("tar", "lzma"),
    "tar.lzma" = list("tar", "lzma"),

    "txz" = list("tar", "xz"),
    "tar.xz" = list("tar", "xz"),

    "tzo" = list("tar", "lzop"),

    "taZ" = list("tar", "compress"),
    "tZ" = list("tar", "compress"),

    "tar.zst" = list("tar", "zstd"),

    "warc" = list("warc", "none"),

    "jar" = list("zip", "none"),
    "zip" = list("zip", "none"),

    NULL)
}

libarchive_version <- function() {
  package_version(libarchive_version_())
}

libarchive_zlib_version <- function() {
  version <- libarchive_zlib_version_()
  if (nzchar(version)) {
    return(package_version(version))
  }
  package_version("0.0.0")
}

libarchive_liblzma_version <- function() {
  version <- libarchive_liblzma_version_()
  if (nzchar(version)) {
    return(package_version(version))
  }
  package_version("0.0.0")
}

libarchive_bzlib_version <- function() {
  version <- libarchive_bzlib_version_()

  # bzlib versions are of the form
  # 1.0.6, 6-Sept-2010
  # So remove everything after the comma
  version <- sub(",.+", "", version)

  if (nzchar(version)) {
    return(package_version(version))
  }
  package_version("0.0.0")
}

libarchive_liblz4_version <- function() {
  version <- libarchive_liblz4_version_()
  if (nzchar(version)) {
    return(package_version(version))
  }
  package_version("0.0.0")
}

libarchive_libzstd_version <- function() {
  version <- libarchive_libzstd_version_()
  if (nzchar(version)) {
    return(package_version(version))
  }
  package_version("0.0.0")
}
