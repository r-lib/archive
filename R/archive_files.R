#' Extract contents of an archive
#'
#' @inheritParams archive_connections
#' @param dir `character(1)` Directory location to extract archive contents, will be created
#' if it does not exist.
#' @details
#' If `file` is `NULL` (the default) all files will be extracted.
#' @examples
#' a <- archive(system.file(package = "archive", "extdata", "data.zip"))
#' d <- tempfile()
#'
#' # When called with default arguments extracts all files in the archive.
#' archive_extract(a, d)
#' list.files(d)
#' unlink(d)
#'
#' # Can also specify one or more files to extract
#' d <- tempfile()
#' archive_extract(a, d, c("iris.csv", "airquality.csv"))
#' list.files(d)
#' unlink(d)
#' @export
archive_extract <- function(archive, dir = ".", file = NULL) {
  archive <- as_archive(archive)
  assert("`file` must be a character or numeric vector or `NULL`",
    is.null(file) || is.numeric(file) || is.character(file))

  if (is.numeric(file)) {
    file <- archive$path[file]
  }

  if (is.null(file)) {
    file <- character()
  }

  if (!identical(dir, ".")) {
    if (!dir.exists(dir)) {
      dir.create(dir)
    }
    old <- setwd(dir)
    on.exit(setwd(old))
  }
  archive_extract_(attr(archive, "path"), file)

  invisible(archive)
}

#' Add files to a new archive
#'
#' `archive_write_files()` adds one or more files to a new archive.
#' `archive_write_dir()` adds all the file(s) in a directory to a new archive.
#' @param files `character()` One or more files to add to the archive.
#' @inheritParams archive_connections
#' @examples
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
#' @export
archive_write_files <- function(archive, files, format = NULL, filter = NULL) {
  assert("`archive` must be a writable file path",
    is_writable(dirname(archive)))

  assert("`files` must be one or more readable file paths",
    lapply(files, is_readable))

  if (is.null(format) && is.null(filter)) {
    res <- format_and_filter_by_extension(archive)
    assert("Could not automatically determine the `filter` and `format`",
      non_null(res))
    format <- res[[1]]
    filter <- res[[2]]
  }
  write_files_(archive, files, archive_formats()[format], archive_filters()[filter])

  invisible(as_archive(archive))
}

#' @rdname archive_write_files
#' @param ... additional paramters passed to `base::dir`
#' @param dir `character(1)` The directory of files to add.
#' @inheritParams base::list.files
#' @export
archive_write_dir <- function(archive, dir, ..., recursive = TRUE, full.names = FALSE) {
  assert("`dir` is not readable",
    is_readable(dir))

  archive <- file.path(normalizePath(dirname(archive)), basename(archive))
  old <- setwd(dir)
  on.exit(setwd(old))
  files <- dir(dir, ..., recursive = recursive, full.names = full.names)
  archive_write_files(archive, files)

  invisible(as_archive(archive))
}
