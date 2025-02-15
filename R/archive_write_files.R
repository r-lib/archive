#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files `character()` One or more files to add to the archive.
#' @inheritParams archive_write
#' @returns An 'archive' object representing the new archive (invisibly).
#' @examples
#' if (archive:::libarchive_version() > "3.2.0") {
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
#'
#' unlink("data.zip")
#' }
#' @export
archive_write_files <- function(archive, files, format = NULL, filter = NULL, options = character(), password = NA_character_) {
  assert("`archive` {archive} must be a writable file path",
    is_writable(dirname(archive)))

  archive <- normalizePath(archive, mustWork = FALSE)

  assert("`files` must be one or more readable file paths",
    lapply(files, is_readable))

  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)
    assert("Could not automatically determine the `filter` and `format` from `archive` {archive}",
      non_null(res))
    format <- res[[1]]
    filter <- res[[2]]
  }
  options <- validate_options(options)

  archive_write_files_(archive, files, archive_formats()[format], archive_filters()[filter], options, c(password), sz = 2^14)

  invisible(archive(archive, options = character()))
}
