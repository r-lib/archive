choices_rd <- function(x) {
  paste0(collapse = ", ", paste0("\\sQuote{", x, "}")) # nocov
}

assert <- function(msg, ...) {
  tests <- unlist(list(...))

  if (!all(tests)) {
    stop(structure(list(message = msg, .call = sys.call(-1)), class = c("error", "condition")))
  }
}

is_string <- function(x) {
  is.character(x) && length(x) == 1
}

is_readable <- function(path) {
  is_string(path) &&
    file.exists(path) &&
    file.access(path, mode = 4)[[1]] == 0
}

is_writable <- function(path) {
  is_string(path) &&
    file.exists(path) &&
    file.access(path, mode = 2)[[1]] == 0
}

non_null <- function(x) {
  !is.null(x)
}

is_number <- function(x) {
  is.numeric(x) && length(x) == 1
}
