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
