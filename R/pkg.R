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

#' Construct a write only connection into an archive
#'
#' @param archive The archive filename
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
