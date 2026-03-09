# Extract contents of an archive to a directory

Extract contents of an archive to a directory

## Usage

``` r
archive_extract(
  archive,
  dir = ".",
  files = NULL,
  options = character(),
  strip_components = 0L,
  password = NA_character_
)
```

## Arguments

- archive:

  `character(1)` The archive filename or an `archive` object.

- dir:

  `character(1)` Directory location to extract archive contents, will be
  created if it does not exist.

- files:

  `character() || integer() || NULL` One or more files within the
  archive, specified either by filename or by position.

- options:

  [`character()`](https://rdrr.io/r/base/character.html) default:
  `character(0)` Options to pass to the filter or format. The list of
  available options are documented in options can have one of the
  following forms:

  - `option=value` The option/value pair will be provided to every
    module. Modules that do not accept an option with this name will
    ignore it.

  - `option` The option will be provided to every module with a value of
    "1".

  - `!option` The option will be provided to every module with a NULL
    value.

  - `module:option=value`, `module:option`, `module:!option` As above,
    but the corresponding option and value will be provided only to
    modules whose name matches module. See [read
    options](https://man.freebsd.org/cgi/man.cgi?query=archive_read_set_options&sektion=3&format=html)
    for available read options See [write
    options](https://man.freebsd.org/cgi/man.cgi?query=archive_write_set_options&sektion=3&format=html)
    for available write options

- strip_components:

  Remove the specified number of leading path elements. Pathnames with
  fewer elements will be silently skipped.

- password:

  `character(1)` The password to process the archive.

## Value

The filenames extracted (invisibly).

## Details

If `files` is `NULL` (the default) all files will be extracted.

## Examples

``` r
a <- system.file(package = "archive", "extdata", "data.zip")
d <- tempfile()

# When called with default arguments extracts all files in the archive.
archive_extract(a, d)
list.files(d)
#> [1] "airquality.csv" "iris.csv"       "mtcars.csv"    
unlink(d)

# Can also specify one or more files to extract
d <- tempfile()
archive_extract(a, d, c("iris.csv", "airquality.csv"))
list.files(d)
#> [1] "airquality.csv" "iris.csv"      
unlink(d)
```
