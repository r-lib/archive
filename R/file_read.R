#' @rdname file_connections
#' @inheritParams archive_read
#' @export
file_read <- function(file, mode = "r") {
  assert("{file} is not a readable file path",
    is_readable(file))

  file_read_(file, mode, sz = 2^14)
}
