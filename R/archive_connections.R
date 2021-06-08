#' @rdname archive_connections
#' @param mode `character(1)` A description of how to open the
#'   connection (if it should be opened initially).  See section
#'   ‘Modes’ in [base::connections()] for possible values.
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

  assert(paste0("`file` {file} not found in `archive` {archive}"),
    file %in% archive$path)

  read_connection(attr(archive, "path"), mode = mode, file, archive_formats()[format], archive_filters()[filter], sz = 2^14)
}

#' Acquire a read or write connection to an archive file
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

    assert("Could not automatically determine the `filter` and `format` from `archive` {archive}",
      !is.null(res))

    format <- res[[1]]
    filter <- res[[2]]
  }

  assert("`archive` {archive} must be a writable file path",
    is_writable(dirname(archive)))

  assert("`file` must be a length one character vector",
    is_string(file))

  write_connection(archive, file, archive_formats()[format], archive_filters()[filter], sz = 2^14)
}
