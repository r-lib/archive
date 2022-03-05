#' @rdname archive_write_files
#' @param ... additional parameters passed to `base::dir`.
#' @param dir `character(1)` The directory of files to add.
#' @inheritParams base::list.files
#' @returns An 'archive' object representing the new archive (invisibly).
#' @export
archive_write_dir <- function(archive, dir, format = NULL, filter = NULL, options = character(), password = "", ..., recursive = TRUE, full.names = FALSE) {
  assert("`dir` {dir} is not readable",
    is_readable(dir))

  archive <- file.path(normalizePath(dirname(archive)), basename(archive))

  options <- validate_options(options)

  old <- setwd(dir)
  on.exit(setwd(old))
  files <- dir(".", ..., recursive = recursive, full.names = full.names)

  archive_write_files(archive, files, format = format, filter = filter, options = options, password = password)

  invisible(archive(archive, options = character()))
}
