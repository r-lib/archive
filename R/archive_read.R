#' Create a readable connection to a file in an archive.
#'
#' @inheritParams archive_write
#' @returns An 'archive_read' connection to the file within the archive to be read.
#' @examples
#' a <- system.file(package = "archive", "extdata", "data.zip")
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
  assert("`file` must be a length one character vector or numeric",
    length(file) == 1 && (is.character(file) || is.numeric(file)))

  options <- validate_options(options)

  if (!inherits(archive, "connection")) {
    archive <- file(archive)
  }

  description <- glue::glue("archive_read({desc})[{file}]", desc = summary(archive)$description)

  archive_read_(archive, file, description, mode, archive_formats()[format], archive_filters()[filter], options, sz = 2^14)
}
