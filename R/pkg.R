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
#' @seealso [archive_read()], [archive_write()] to read and write archive files
#' using R connections, [archive_extract()], [archive_write_files()],
#' [archive_write_dir()] to add or extract files from an archive.
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

#' @rdname archive_connections
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' # Show files in archive
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

#' Aquire a read or write connection to an archive file
#'
#' `archive_read()` returns an readable input connection to an existing archive.
#' `archive_write()` returns an writable output connection to a new archive.
#'
#' @param archive `character(1)` The archive filename or an `archive` object.
#' @param file `character(1) || integer(1)` The filename within the archive,
#'   specified either by filename or by position.
#' @name archive_connections
#' @template archive
#' @details
#' If `format` and `filter` are `NULL`, they will be set automatically based on
#' the file extension given in `file` for `archive_write()` or automatically
#' detected using
#' [Robust automatic format detection](https://github.com/libarchive/libarchive/wiki/FormatDetection)
#' for `archive_read()`.
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

#' Extract contents of an archive
#'
#' @inheritParams archive_connections
#' @param dir `character(1)` Directory location to extract archive contents, will be created
#' if it does not exist.
#' @details
#' If `file` is `NULL` (the default) all files will be extracted.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' d <- tempfile()
#'
#' # When called with default arguments extracts all files in the archive.
#' archive_extract(a, d)
#' list.files(d)
#' unlink(d)
#'
#' # Can also specify one or more files to extract
#' d <- tempfile()
#' archive_extract(a, d, c("iris.csv", "airquality.csv"))
#' list.files(d)
#' unlink(d)
#' @export
archive_extract <- function(archive, dir = ".", file = NULL) {
  archive <- as_archive(archive)
  assert("`file` must be a character or numeric vector or `NULL`",
    is.null(file) || is.numeric(file) || is.character(file))

  if (is.numeric(file)) {
    file <- archive$path[file]
  }

  if (is.null(file)) {
    file <- character()
  }

  if (!identical(dir, ".")) {
    if (!dir.exists(dir)) {
      dir.create(dir)
    }
    old <- setwd(dir)
    on.exit(setwd(old))
  }
  archive_extract_(attr(archive, "path"), file)

  invisible(archive)
}

#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files `character()` One or more files to add to the archive.
#' @inheritParams archive_connections
#' @examples
#' # write some files to a directory
#' d <- tempfile()
#' dir.create(d)
#' old <- setwd(d)
#'
#' write.csv(iris, file.path(d, "iris.csv"))
#' write.csv(mtcars, file.path(d, "mtcars.csv"))
#' write.csv(airquality, file.path(d, "airquality.csv"))
#'
#' # Add some to a new archive
#' a <- archive_write_files("data.tar.gz", c("iris.csv", "mtcars.csv"))
#' setwd(old)
#' a
#'
#' # Add all files in a directory
#' a <- archive_write_dir("data.zip", d)
#' a
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

  invisible(as_archive(archive))
}

#' @rdname archive_write_files
#' @param ... additional paramters passed to `base::dir`
#' @param dir `character(1)` The directory of files to add.
#' @inheritParams base::list.files
#' @export
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = FALSE) {
  assert("`dir` is not readable",
    is_readable(dir))

  archive <- file.path(normalizePath(dirname(archive)), basename(archive))
  old <- setwd(dir)
  on.exit(setwd(old))
  files <- dir(dir, ..., recursive = recursive, full.names = full.names)
  archive_write_files(archive, files)

  invisible(as_archive(archive))
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

#' Construct a connections for (possibly compressed) files.
#'
#' These work similar to R's built-in [connections] for files and differ from
#' [archive_read] and [archive_write] because they do not use an archive
#' format, just use one or more of the filters.
#'
#' `file_write()` returns an writable output connection,
#' `file_read()` returns a readable input connection.
#' @inheritParams archive_connections
#' @name file_connections
#' @export
#' @examples
#' # Write bzip2, base 64 encoded data
#' write.csv(mtcars, file_write("mtcars.bz2", c("uuencode", "bzip2")))
#'
#' # Read it back
#' read.csv(file_read("mtcars.bz2"), row.names = 1, nrows = 3)
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
