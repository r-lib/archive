# Construct a new archive

This function retrieves metadata about files in an archive, it can be
passed to
[`archive_read()`](https://archive.r-lib.org/reference/archive_read.md)
or [archive_write](https://archive.r-lib.org/reference/archive_write.md)
to create a connection to read or write a specific file from the
archive.

## Usage

``` r
archive(file, options = character(), password = NA_character_)
```

## Arguments

- file:

  File path to the archive.

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

A [tibble](https://tibble.tidyverse.org/reference/tibble-package.html)
with details about files in the archive.

## See also

[`archive_read()`](https://archive.r-lib.org/reference/archive_read.md),
[`archive_write()`](https://archive.r-lib.org/reference/archive_write.md)
to read and write archive files using R connections,
[`archive_extract()`](https://archive.r-lib.org/reference/archive_extract.md),
[`archive_write_files()`](https://archive.r-lib.org/reference/archive_write_files.md),
[`archive_write_dir()`](https://archive.r-lib.org/reference/archive_write_files.md)
to add or extract files from an archive.

## Examples

``` r
a <- archive(system.file(package = "archive", "extdata", "data.zip"))
a
#> # A tibble: 3 × 3
#>   path            size date               
#>   <chr>          <int> <dttm>             
#> 1 iris.csv         192 2017-04-28 19:55:29
#> 2 mtcars.csv       274 2017-04-28 19:55:29
#> 3 airquality.csv   142 2017-04-28 19:55:29
```
