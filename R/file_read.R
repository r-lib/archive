#' @rdname file_connections
#' @export
file_read <- function(file, mode = "r", filter = NULL, options = character(), password = NA_character_) {
  options <- validate_options(options)

  if (!inherits(archive, "connection")) {
    file <- file(file)
  }

  description <- glue::glue("file_read({desc})", desc = summary(file)$description)

  archive_read_(file, 1L, description, mode, archive_formats()["raw"], archive_filters()[filter], options, c(password), sz = 2^14)
}
