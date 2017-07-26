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
#' @export
archive <- function(path) {
  stopifnot(is.character(path), length(path) == 1, file.exists(path))

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

#' Construct a read only connection into an archive file
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
#' @export
archive_read <- function(archive, file = 1L, mode = "r", format = NULL, filter = NULL) {
  archive <- as_archive(archive)
  if (is.numeric(file) && length(file) == 1) {
    file <- archive$path[[file]]
  }
  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector or numeric", call. = FALSE)
  }

  if (!file %in% archive$path) {
    stop("`file` ", encodeString(file, quote = "'"), " not found", call. = FALSE)
  }

  read_connection(attr(archive, "path"), mode = mode, file, archive_formats()[format], archive_filters()[filter])
}

#' Construct a write only connection to a new archive
#'
#' @param archive The archive filename, if filter and format are both `NULL` the file extension will
#' be used to determine the archive format and filters, if any.
#' @param format `character(1)` default: `NULL` The archive format, one of \Sexpr[stage=render, results=rd]{archive:::choices_rd(names(archive:::archive_formats()))}.
#' @param filter `character(1)` default: `NULL` The archive filter, one of \Sexpr[stage=render, results=rd]{archive:::choices_rd(names(archive:::archive_filters()))}.
#' @param file The filename within the archive.
#' @export
archive_write <- function(archive, file, format = NULL, filter = NULL) {
  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)
    if (is.null(res)) {
      stop("Could not automatically determine the `filter` and `format`", call. = FALSE)
    }
    format <- res[[1]]
    filter <- res[[2]]
  }
  if (!is.character(archive) || length(archive) != 1) {
    stop("`archive` must be a length one character vector", call. = FALSE)
  }

  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector", call. = FALSE)
  }
  write_connection(archive, file, archive_formats()[format], archive_filters()[filter])
}

#' Construct a file connections for (possibly compressed) files.
#'
#' This works similar to R's builtin [connections] for files. However it
#' supports one on or more of the following
#'
#' `file_write()` returns an writable output connection,
#' `file_read()` returns a readable input connection.
#' @inheritParams archive_write
#' @name file_connections
#' @export
file_write <- function(file, filter = NULL) {
  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector", call. = FALSE)
  }
  if (is.null(filter)) {
    res <- filter_by_extension(file)
    if (is.null(res)) {
      stop("Could not automatically determine the `filter`", call. = FALSE)
    }
    filter <- res
  }

  write_file_connection(file, archive_filters()[filter])
}

#' @rdname file_connections
#' @inheritParams archive_read
#' @export
file_read <- function(file, mode = "r") {
  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector", call. = FALSE)
  }

  read_file_connection(file, mode)
}

#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files One or more files to add to the archive.
#' @inheritParams archive_write
#' @export
archive_write_files <- function(archive, files, format = NULL, filter = NULL) {
  if (!is.character(archive) || length(archive) != 1) {
    stop("`archive` must be a length one character vector", call. = FALSE)
  }

  if (!is.character(files) || length(files) < 1) {
    stop("`file` must be a length 1 or greater character vector", call. = FALSE)
  }

  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)
    if (is.null(res)) {
      stop("Could not automatically determine the `filter` and `format`", call. = FALSE)
    }
    format <- res[[1]]
    filter <- res[[2]]
  }
  write_files_(archive, files, archive_formats()[format], archive_filters()[filter])

  invisible(archive)
}

#' @rdname archive_write
#' @param ... additional paramters passed to `base::dir`
#' @param dir The directory of files to add
#' @inheritParams base::list.files
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = FALSE) {
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
