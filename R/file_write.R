#' Construct a connections for (possibly compressed) files.
#'
#' They are functionally equivalent to calling [archive_read] or
#' [archive_write] using `format = "raw", archive = file`.
#'
#' `file_write()` returns an writable output connection,
#' `file_read()` returns a readable input connection.
#' @inheritParams archive_read
#' @inheritParams archive_write
#' @name file_connections
#' @export
#' @examples
#' if (archive:::libarchive_version() > "3.2.0") {
#' # Write bzip2, base 64 encoded data and use high compression
#' write.csv(mtcars,
#'   file_write("mtcars.bz2",
#'     filter = c("uuencode", "bzip2"),
#'     options = "compression-level=9"
#'   )
#' )
#'
#' # Read it back
#' read.csv(file_read("mtcars.bz2"), row.names = 1, nrows = 3)
#' unlink("mtcars.bz2")
#' }
file_write <- function(file, mode = "w", filter = NULL, options = character()) {

  if (is.null(filter)) {
    res <- filter_by_extension(file)
    assert("Could not automatically determine the `filter` for {file}",
      non_null(res))
    filter <- res
  }

  archive_write(archive = file, file = file, mode = mode, format = "raw", filter = filter, options = options)
}
