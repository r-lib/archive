#' @useDynLib archive
#' @importFrom Rcpp sourceCpp
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
  stopifnot(file.exists(path))

  path <- normalizePath(path)

  res <- archive_metadata(path)
  class(res) <- c("archive", class(res))
  res
}

#' Construct a read only connection into an archive
#'
#' @param archive An archive object or character vector to the archive
#' @param file The file to open the connection to. Can also be an numeric index
#' into the `archive()` data.frame.
#' @export
archive_read <- function(archive, file = 1L) {
  if (is.character(archive) && length(archive) == 1) {
    archive <- archive(archive)
  }
  if (!inherits(archive, "archive")) {
    stop("`archive` must be a archive object or character", call. = FALSE)
  }
  if (is.numeric(file) && length(file) == 1) {
    file <- archive$path[[file]]
  }
  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector or numeric", call. = FALSE)
  }

  if (!file %in% archive$path) {
    stop("`file` ", encodeString(file, quote = "'"), " not found", call. = FALSE)
  }

  read_connection(attr(archive, "path"), file)
}

#' Construct a write only connection to a new archive
#'
#' @param archive The archive filename, the extension(s) given will
#' automatically determine the archive type and compression used.
#' @param file The filename within the archive.
#' @export
archive_write <- function(archive, file) {
  if (!is.character(archive) || length(archive) != 1) {
    stop("`archive` must be a length one character vector", call. = FALSE)
  }

  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector", call. = FALSE)
  }
  write_connection(archive, file)
}

#' Construct a write only connection to a (possibly compressed) file.
#'
#' This works similar to R's builtin [connections]. However it supports one on
#' or more of.
#'
#' - uuencode
#' - gzip compression
#' - bzip2 compression
#' - compress/LZW compression
#' - lzma, lzip, and xz compression
#' - lz4 compression
#' - lzop compression
#' @inheritParams archive_write
#' @param type The type of filters to use for the file.
#' @export
file_write <- function(file, type) {
  if (!is.character(file) || length(file) != 1) {
    stop("`file` must be a length one character vector", call. = FALSE)
  }

  write_file_connection(file, type)
}

#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param archive The archive filename, extension will automatically determine
#' the archive format and filters (if any).
#' @param One or more files to add to the archive.
#' @export
archive_write_files <- function(archive, files) {
  if (!is.character(archive) || length(archive) != 1) {
    stop("`archive` must be a length one character vector", call. = FALSE)
  }

  if (!is.character(files) || length(files) < 1) {
    stop("`file` must be a length 1 or greater character vector", call. = FALSE)
  }
  write_files(archive, files)

  invisible(archive)
}

#' @rdname archive_write
#' @param ... additional paramters passed to `base::dir`
#' @inheritParams base::list.files
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = TRUE) {
  files <- dir(..., recursive = recursive, full.names = full.names)
  archive_write_files(archive, files)
}
