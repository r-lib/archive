#' Create a writable connection to a file in an archive.
#'
#' @param archive `character(1)` The archive filename or an `archive` object.
#' @param file `character(1) || integer(1)` The filename within the archive,
#'   specified either by filename or by position.
#' @template archive
#' @importFrom rlang is_character is_named
#' @examples
#' # Archive format and filters can be set automatically from the file extensions.
#' f1 <- tempfile(fileext = ".tar.gz")
#'
#' write.csv(mtcars, archive_write(f1, "mtcars.csv"))
#' archive(f1)
#' unlink(f1)
#'
#' # They can also be specified explicitly
#' f2 <- tempfile()
#' write.csv(mtcars, archive_write(f2, "mtcars.csv", format = "tar", filter = "bzip2"))
#' archive(f2)
#' unlink(f2)
#'
#' # You can also pass additional options to control things like compression level
#' f3 <- tempfile(fileext = ".tar.gz")
#' write.csv(mtcars, archive_write(f3, "mtcars.csv", options = "compression-level=2"))
#' archive(f3)
#' unlink(f3)
#' @export
archive_write <- function(archive, file, format = NULL, filter = NULL, options = character()) {
  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)

    assert("Could not automatically determine the `filter` and `format` from `archive` {archive}",
      !is.null(res))

    format <- res[[1]]
    filter <- res[[2]]
  }

  assert("`archive` {archive} must be a writable file path",
    is_writable(dirname(archive)))

  archive <- normalizePath(archive, mustWork = FALSE)

  assert("`file` must be a length one character vector",
    is_string(file))

  options <- validate_options(options)

  archive_write_(archive, file, archive_formats()[format], archive_filters()[filter], options, 2^14)
}
