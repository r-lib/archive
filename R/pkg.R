#' @useDynLib archive, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @importFrom tibble as_tibble
NULL

#' Construct a new archive
#'
#' This function retrieves metadata about files in an archive, it can be passed
#' to `archive_con()` to create a connection to read a specific file from the
#' archive.
#'
#' @param path File path to the archive.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' a
#' @export
archive <- function(path) {
  assert("`path` must be a readable file path",
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

#' Extract contents of an archive
#'
#' @param archive An archive object or file path to the archive location.
#' @param dir Directory location to extract archive contents, will be created
#' if it does not exist.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' d <- tempfile()
#' dir.create(d)
#' archive_extract(a, d)
#' list.files(d)
#' unlink(d)
#' @export
archive_extract <- function(archive, dir = ".") {
  archive <- as_archive(archive)

  if (!identical(dir, ".")) {
    if (!dir.exists(dir)) {
      dir.create(dir)
    }
    old <- setwd(dir)
    on.exit(setwd(old))
  }
  archive_extract_(attr(archive, "path"))
}

#' Construct a read or write connection into an archive file
#'
#' @param archive An archive object or character vector to the archive
#' @param file The file to open the connection to. Can also be an numeric index
#' into the `archive()` data.frame.
#' @param mode The mode to open the file in.
#' @inheritParams archive_write
#' @details
#' libarchive versions prior to 3.1.0 did not support explicit setting of the
#' format and filter, instead relying on a bidding process to automatically
#' determine the format of the archive. This automatic detection is also used
#' when `format` or `filter` is `NULL`.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' # See files in archive
#' a
#'
#' # By default reads the first file in the archive.
#' read.csv(archive_read(a), nrows = 3)
#'
#' # Can also specify a filename directly
#' read.csv(archive_read(a, "mtcars.csv"), nrows = 3)
#'
#' # Or by position
#' read.csv(archive_read(a, 3), nrows = 3)
#'
#' # Explicitly specify the format and filter if automatic detection fails.
#' read.csv(archive_read(a, format = "zip"), nrows = 3)
#' @export
archive_read <- function(archive, file = 1L, mode = "r", format = NULL, filter = NULL) {
  archive <- as_archive(archive)
  if (is_number(file)) {
    file <- archive$path[[file]]
  }

  assert("`file` must be a length one character vector or numeric",
    length(file) == 1 && (is.character(file) || is.numeric(file)))

  assert(paste0("`file` ", encodeString(file, quote = "'"), " not found"),
    file %in% archive$path)

  read_connection(attr(archive, "path"), mode = mode, file, archive_formats()[format], archive_filters()[filter])
}

#' Construct a write only connection to a new archive
#'
#' `archive_read()` returns an readable input connection to an existing archive.
#' `archive_write()` returns an writable output connection to a new archive.
#'
#' @param archive `character(1)` The archive filename.
#' @param format `character(1)` default: `NULL` The archive format, one of \Sexpr[stage=render, results=rd]{archive:::choices_rd(names(archive:::archive_formats()))}.
#' @param filter `character(1)` default: `NULL` The archive filter, one of \Sexpr[stage=render, results=rd]{archive:::choices_rd(names(archive:::archive_filters()))}.
#' @param file `character(1)` The filename within the archive.
#' @details
#' If `format` and `filter` are `NULL`, they will be set automatically based on
#' the file extension given in `file`.
#'
#' @examples
#' # Achive format and filters can be set automatically from the file extensions.
#' f1 <- tempfile(fileext = ".tar.gz")
#'
#' write.csv(mtcars, archive_write(f1, "mtcars.csv"))
#' archive(f1)
#' unlink(f1)
#'
#' # They can also be specified explicitly
#' f2 <- tempfile()
#' write.csv(iris, archive_write(f2, "iris.csv", format = "tar", filter = "bzip2"))
#' archive(f2)
#' unlink(f2)
#' @export
archive_write <- function(archive, file, format = NULL, filter = NULL) {
  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)

    assert("Could not automatically determine the `filter` and `format`",
      !is.null(res))

    format <- res[[1]]
    filter <- res[[2]]
  }

  assert("`archive` must be a writable file path",
    is_writable(dirname(archive)))

  assert("`file` must be a length one character vector",
    is_string(file))

  write_connection(archive, file, archive_formats()[format], archive_filters()[filter])
}

#' Construct a connections for (possibly compressed) files.
#'
#' These work similar to R's built-in [connections] for files and differ from
#' [archive_read] and [archive_write] because they do not use an archive
#' format, just use one or more of the filters.
#'
#' `file_write()` returns an writable output connection,
#' `file_read()` returns a readable input connection.
#' @inheritParams archive_write
#' @name file_connections
#' @export
file_write <- function(file, filter = NULL) {
  assert("`file` must be a writable file path",
    is_writable(dirname(file)))

  if (is.null(filter)) {
    res <- filter_by_extension(file)
    assert("Could not automatically determine the `filter`",
      non_null(res))
    filter <- res
  }

  write_file_connection(file, archive_filters()[filter])
}

#' @rdname file_connections
#' @inheritParams archive_read
#' @export
file_read <- function(file, mode = "r") {
  assert("`file` must be a readable file path",
    is_readable(file))

  read_file_connection(file, mode)
}

#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files `[character()]` One or more files to add to the archive.
#' @inheritParams archive_write
#' @export
archive_write_files <- function(archive, files, format = NULL, filter = NULL) {
  assert("`archive` must be a writable file path",
    is_writable(dirname(archive)))

  assert("`files` must be one or more readable file paths",
    lapply(files, is_readable))

  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)
    assert("Could not automatically determine the `filter` and `format`",
      non_null(res))
    format <- res[[1]]
    filter <- res[[2]]
  }
  write_files_(archive, files, archive_formats()[format], archive_filters()[filter])

  invisible(archive)
}

#' @rdname archive_write
#' @param ... additional paramters passed to `base::dir`
#' @param dir [character(1)] The directory of files to add.
#' @inheritParams base::list.files
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = FALSE) {
  assert("`dir` is not readable",
    is_readable(dir))

  archive <- file.path(normalizePath(dirname(archive)), basename(archive))
  old <- setwd(dir)
  on.exit(setwd(old))
  files <- dir(dir, ..., recursive = recursive, full.names = full.names)
  archive_write_files(archive, files)
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

      NULL)
  }

  extensions <- sub("^[^.][.]", "", basename(path))

  Reduce(`c`, Map(extension_to_filter, strsplit(extensions, "[.]")[[1]]))
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

    "warc" = list("warc", "none"),

    "jar" = list("zip", "none"),
    "zip" = list("zip", "none"),

    NULL)
}
