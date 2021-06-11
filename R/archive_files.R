#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files `character()` One or more files to add to the archive.
#' @inheritParams archive_write
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
archive_write_files <- function(archive, files, format = NULL, filter = NULL, options = character()) {
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

  write_files_(archive, files, archive_formats()[format], archive_filters()[filter], options, sz = 2^14)

  invisible(as_archive(archive))
}

#' @rdname archive_write_files
#' @param ... additional parameters passed to `base::dir`.
#' @param dir `character(1)` The directory of files to add.
#' @inheritParams base::list.files
#' @export
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = FALSE) {
  assert("`dir` {dir} is not readable",
    is_readable(dir))

  archive <- file.path(normalizePath(dirname(archive)), basename(archive))
  old <- setwd(dir)
  on.exit(setwd(old))
  files <- dir(dir, ..., recursive = recursive, full.names = full.names)
  archive_write_files(archive, files)

  invisible(as_archive(archive))
}
