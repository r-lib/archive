## usethis namespace: start
#' @useDynLib archive, .registration = TRUE
#' @importFrom tibble as_tibble
## usethis namespace: end
NULL

#' Construct a new archive
#'
#' This function retrieves metadata about files in an archive, it can be passed
#' to `archive_con()` to create a connection to read a specific file from the
#' archive.
#'
#' @param path File path to the archive.
#' @seealso [archive_read()], [archive_write()] to read and write archive files
#' using R connections, [archive_extract()], [archive_write_files()],
#' [archive_write_dir()] to add or extract files from an archive.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' a
#' @export
archive <- function(path) {
  assert("{path} is not a readable file path",
    is_readable(path))

  path <- normalizePath(path)

  res <- archive_metadata(path)
  class(res) <- c("archive", class(res))
  res
}

as_archive <- function(x) {
  if (inherits(x, "archive")) {
    return(x)
  }
  archive(x)
}


filter_by_extension <- function(path) {

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

  extensions <- sub("^[^.][.]", "", basename(path))

  Reduce(`c`, Map(extension_to_filter, strsplit(extensions, "[.]")[[1]]))
}

#' Construct a connections for (possibly compressed) files.
#'
#' These work similar to R's built-in [connections] for files and differ from
#' [archive_read] and [archive_write] because they do not use an archive
#' format, just use one or more of the filters.
#'
#' `file_write()` returns an writable output connection,
#' `file_read()` returns a readable input connection.
#' @inheritParams archive_read
#' @inheritParams archive_write
#' @name file_connections
#' @export
#' @examples
#' if (archive:::libarchive_version() > "3.2.0") {
#' # Write bzip2, base 64 encoded data
#' write.csv(mtcars, file_write("mtcars.bz2", c("uuencode", "bzip2")))
#'
#' # Read it back
#' read.csv(file_read("mtcars.bz2"), row.names = 1, nrows = 3)
#' unlink("mtcars.bz2")
#' }
file_write <- function(file, filter = NULL, options = character()) {
  assert("{file} is not a writable file path",
    is_writable(dirname(file)))

  file <- normalizePath(file, mustWork = FALSE)

  if (is.null(filter)) {
    res <- filter_by_extension(file)
    assert("Could not automatically determine the `filter` for {file}",
      non_null(res))
    filter <- res
  }

  options <- validate_options(options)

  write_file_connection(file, archive_filters()[filter], options)
}

#' @rdname file_connections
#' @inheritParams archive_read
#' @export
file_read <- function(file, mode = "r") {
  assert("{file} is not a readable file path",
    is_readable(file))

  read_file_connection(file, mode, sz = 2^14)
}

format_and_filter_by_extension <- function(path) {
  ext <- sub("^[^.]*[.]", "", basename(path))
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
