choices_rd <- function(x) {
  paste0(collapse = ", ", paste0("\\sQuote{", x, "}")) # nocov
}

#' @importFrom glue collapse single_quote evaluate
collapse_quote_transformer <- function(code, envir, data) {
  collapse_re <- "[*]$"
  quote_re <- "^[|]"
  should_collapse <- grepl(collapse_re, code)
  should_quote <- !grepl(quote_re, code)
  code <- sub(collapse_re, "",
    sub(quote_re, "", code))
  res <- evaluate(code, envir, data)
  if (should_quote) {
    res <- single_quote(res)
  }
  if (should_collapse) {
    res <- collapse(res, sep = ", ", last = " and ")
  }
  res
}

#' @importFrom glue glue
assert <- function(msg, ...) {
  tests <- unlist(list(...))

  if (!all(tests)) {
    stop(structure(list(
          message = glue(msg, .envir = parent.frame(), .transformer = collapse_quote_transformer),
          .call = sys.call(-1)), class = c("error", "condition")))
  }
}

is_string <- function(x) {
  is.character(x) && length(x) == 1
}

is_readable <- function(path) {
  is_string(path) &&
    file.exists(path)
    # file.access fails on some NFS, such as shared folders on virtualbox
    # https://stat.ethz.ch/pipermail/r-devel/2008-December/051461.html
    # file.access(path, mode = 4)[[1]] == 0
}

is_writable <- function(path) {
  is_string(path) &&
    file.exists(path)
    # file.access fails on some NFS, such as shared folders on virtualbox
    # https://stat.ethz.ch/pipermail/r-devel/2008-December/051461.html
    # file.access(path, mode = 2)[[1]] == 0
}

non_null <- function(x) {
  !is.null(x)
}

is_number <- function(x) {
  is.numeric(x) && length(x) == 1
}

dir.exists <- function(paths) {
  x <- base::file.info(paths)$isdir
  !is.na(x) & x
}

file.size <- function(...) {
  base::file.info(...)$size
}
