# Construct a connections for (possibly compressed) files.

They are functionally equivalent to calling
[archive_read](https://archive.r-lib.org/reference/archive_read.md) or
[archive_write](https://archive.r-lib.org/reference/archive_write.md)
using `format = "raw", archive = file`.

## Usage

``` r
file_read(
  file,
  mode = "r",
  filter = NULL,
  options = character(),
  password = NA_character_
)

file_write(
  file,
  mode = "w",
  filter = NULL,
  options = character(),
  password = NA_character_
)
```

## Arguments

- file:

  `character(1) || integer(1)` The filename within the archive,
  specified either by filename or by position.

- mode:

  `character(1)` A description of how to open the connection (if it
  should be opened initially). See section ‘Modes’ in
  [`base::connections()`](https://rdrr.io/r/base/connections.html) for
  possible values.

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

An 'archive_read' connection (for `file_read()`) or an 'archive_write'
connection (for `file_write()`) to the file.

## Details

`file_write()` returns an writable output connection, `file_read()`
returns a readable input connection.

## Examples

``` r
if (archive:::libarchive_version() > "3.2.0") {
# Write bzip2, base 64 encoded data and use high compression
write.csv(mtcars,
  file_write("mtcars.bz2",
    filter = c("uuencode", "bzip2"),
    options = "compression-level=9"
  )
)

# Read it back
read.csv(file_read("mtcars.bz2"), row.names = 1, nrows = 3)
unlink("mtcars.bz2")
}
```
