# Create a writable connection to a file in an archive.

Create a writable connection to a file in an archive.

## Usage

``` r
archive_write(
  archive,
  file,
  mode = "w",
  format = NULL,
  filter = NULL,
  options = character(),
  password = NA_character_
)
```

## Arguments

- archive:

  `character(1)` The archive filename or an `archive` object.

- file:

  `character(1) || integer(1)` The filename within the archive,
  specified either by filename or by position.

- mode:

  `character(1)` A description of how to open the connection (if it
  should be opened initially). See section ‘Modes’ in
  [`base::connections()`](https://rdrr.io/r/base/connections.html) for
  possible values.

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

## Value

An 'archive_write' connection to the file within the archive to be
written.

## Details

If `format` and `filter` are `NULL`, they will be set automatically
based on the file extension given in `file` when writing and
automatically detected using [Robust automatic format
detection](https://github.com/libarchive/libarchive/wiki/FormatDetection)
when reading.

For traditional zip archives `archive_write()` creates a connection
which writes the data to the specified file directly. For other archive
formats the file size must be known when the archive is created, so the
data is first written to a scratch file on disk and then added to the
archive. This scratch file is automatically removed when writing is
complete.

## Examples

``` r
# Archive format and filters can be set automatically from the file extensions.
f1 <- tempfile(fileext = ".tar.gz")

write.csv(mtcars, archive_write(f1, "mtcars.csv"))
archive(f1)
#> # A tibble: 1 × 3
#>   path        size date               
#>   <chr>      <int> <dttm>             
#> 1 mtcars.csv  1783 2026-04-12 08:53:46
unlink(f1)

# They can also be specified explicitly
f2 <- tempfile()
write.csv(mtcars, archive_write(f2, "mtcars.csv", format = "tar", filter = "bzip2"))
archive(f2)
#> # A tibble: 1 × 3
#>   path        size date               
#>   <chr>      <int> <dttm>             
#> 1 mtcars.csv  1783 2026-04-12 08:53:46
unlink(f2)

# You can also pass additional options to control things like compression level
f3 <- tempfile(fileext = ".tar.gz")
write.csv(mtcars, archive_write(f3, "mtcars.csv", options = "compression-level=2"))
archive(f3)
#> # A tibble: 1 × 3
#>   path        size date               
#>   <chr>      <int> <dttm>             
#> 1 mtcars.csv  1783 2026-04-12 08:53:46
unlink(f3)
```
