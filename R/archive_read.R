#' Create a readable connection to a file in an archive.
#'
#' @inheritParams archive_write
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
archive_read <- function(archive, file = 1L, mode = "r", format = NULL, filter = NULL, options = character()) {
  archive <- as_archive(archive, options)
  if (is_number(file)) {
    file <- archive$path[[file]]
  }

  assert("`file` must be a length one character vector or numeric",
    length(file) == 1 && (is.character(file) || is.numeric(file)))

  assert(paste0("`file` {file} not found in `archive` {archive}"),
    file %in% archive$path)

  options <- validate_options(options)

  archive_read_(attr(archive, "path"), file, mode, archive_formats()[format], archive_filters()[filter], options, sz = 2^14)
}
