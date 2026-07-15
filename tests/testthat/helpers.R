
skip_if_no_encoding <- function(enc) {
  tryCatch({
    iconv("xxx", from = env, to = env)
    TRUE
  }, error = function(err) NULL)
  testthat::skip(paste("No encoding:", enc))
}
