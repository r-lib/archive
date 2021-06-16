#' @rdname file_connections
#' @inheritParams archive_read
#' @export
file_read <- function(file, mode = "r", filter = NULL, options = character()) {
  assert("{file} is not a readable file path",
    is_readable(file))

  archive_read_(file, file, mode, archive_formats()["raw"], archive_filters()[filter], options, sz = 2^14)
  #file_read_(file, mode, sz = 2^14)
}
