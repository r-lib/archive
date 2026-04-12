# Add files to a new archive

`archive_write_files()` adds one or more files to a new archive.
`archive_write_dir()` adds all the file(s) in a directory to a new
archive.

## Usage

``` r
archive_write_dir(
  archive,
  dir,
  format = NULL,
  filter = NULL,
  options = character(),
  password = NA_character_,
  ...,
  recursive = TRUE,
  full.names = FALSE
)

archive_write_files(
  archive,
  files,
  format = NULL,
  filter = NULL,
  options = character(),
  password = NA_character_
)
```

## Arguments

- archive:

  `character(1)` The archive filename or an `archive` object.

- dir:

  `character(1)` The directory of files to add.

- format:

  `character(1)` default: `NULL` The archive format, one of
  `choices_rd(names(archive:::archive_formats()))`. Supported formats
  differ depending on the libarchive version and build.

- filter:

  `character(1)` default: `NULL` The archive filter, one of
  `choices_rd(names(archive:::archive_filters()))`. Supported filters
  differ depending on the libarchive version and build.

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

- password:

  `character(1)` The password to process the archive.

- ...:

  additional parameters passed to
  [`base::dir`](https://rdrr.io/r/base/list.files.html).

- recursive:

  logical. Should the listing recurse into directories?

- full.names:

  a logical value. If `TRUE`, the directory path is prepended to the
  file names to give a relative file path. If `FALSE`, the file names
  (rather than paths) are returned.

- files:

  [`character()`](https://rdrr.io/r/base/character.html) One or more
  files to add to the archive.

## Value

An 'archive' object representing the new archive (invisibly).

An 'archive' object representing the new archive (invisibly).

## Examples

``` r
if (archive:::libarchive_version() > "3.2.0") {
# write some files to a directory
d <- tempfile()
dir.create(d)
old <- setwd(d)

write.csv(iris, file.path(d, "iris.csv"))
write.csv(mtcars, file.path(d, "mtcars.csv"))
write.csv(airquality, file.path(d, "airquality.csv"))

# Add some to a new archive
a <- archive_write_files("data.tar.gz", c("iris.csv", "mtcars.csv"))
setwd(old)
a

# Add all files in a directory
a <- archive_write_dir("data.zip", d)
a

unlink("data.zip")
}
```
