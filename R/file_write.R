#' Construct a connections for (possibly compressed) files.
#'
#' These work similar to R's built-in [connections] for files and differ from
#' [archive_read] and [archive_write] because they do not put the file in an
#' archive, they just use one or more of the filters.
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

  file_write_(file, archive_filters()[filter], options)
}
