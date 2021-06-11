#' Extract contents of an archive
#'
#' @inheritParams archive_read
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
  archive_extract_(attr(archive, "path"), file, sz = 2^14)

  invisible(archive)
}

