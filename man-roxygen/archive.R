#' @param format \code{character(1)} default: \code{NULL} The archive format, one of \eval{choices_rd(names(archive:::archive_formats()))}.
#'   Supported formats differ depending on the libarchive version and build.
#' @param filter \code{character(1)} default: \code{NULL} The archive filter, one of \eval{choices_rd(names(archive:::archive_filters()))}.
#'   Supported filters differ depending on the libarchive version and build.
#' @param options \code{character()} default: \code{character(0)} Options to pass to the filter or format.
#'   The list of available options are documented in
#'   options can have one of the following forms:
#'   - `option=value`
#'     The option/value pair will be provided to every module.
#'     Modules that do not accept an option with this name will
#'     ignore it.
#'   - `option`
#'     The option will be provided to every module with a value
#'     of "1".
#'   - `!option`
#'     The option will be provided to every module with a NULL
#'     value.
#'   - `module:option=value`, `module:option`, `module:!option`
#'     As above, but the corresponding option and value will be
#'     provided only to modules whose name matches module.
#'  See [read options](https://man.freebsd.org/cgi/man.cgi?query=archive_read_set_options&sektion=3&format=html) for available read options
#'  See [write options](https://man.freebsd.org/cgi/man.cgi?query=archive_write_set_options&sektion=3&format=html) for available write options
#' @param password `character(1)` The password to process the archive. 
#' @details
#' If `format` and `filter` are `NULL`, they will be set automatically based on
#' the file extension given in `file` when writing and automatically detected
#' using
#' [Robust automatic format detection](https://github.com/libarchive/libarchive/wiki/FormatDetection)
#' when reading.
