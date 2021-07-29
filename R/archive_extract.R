#' Extract contents of an archive to a directory
#'
#' @inheritParams archive_read
#' @param files `character() || integer() || NULL` One or more files within the archive,
#'   specified either by filename or by position.
#' @param dir `character(1)` Directory location to extract archive contents, will be created
#' if it does not exist.
#' @param strip_components Remove the specified number of leading path
#'   elements. Pathnames with fewer elements will be silently skipped.
#' @details
#' If `files` is `NULL` (the default) all files will be extracted.
#' @returns An 'archive' object describing the archive (invisibly).
#' @examples
#' a <- system.file(package = "archive", "extdata", "data.zip")
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
archive_extract <- function(archive, dir = ".", files = NULL, options = character(), strip_components = 0L) {
  assert("`files` must be a character or numeric vector or `NULL`",
    is.null(files) || is.numeric(files) || is.character(files))

  if (!inherits(archive, "connection")) {
    archive <- file(archive, "rb")
  }

  if (!isOpen(archive)) {
    open(archive, "rb")
  }

  if (!identical(dir, ".")) {
    if (!dir.exists(dir)) {
      dir.create(dir)
    }
    old <- setwd(dir)
    on.exit(setwd(old))
  }
  options <- validate_options(options)

  archive_extract_(archive, files, as.integer(strip_components), options, sz = 2^14)

  invisible()
}
