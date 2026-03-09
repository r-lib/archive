# Create a readable connection to a file in an archive.

Create a readable connection to a file in an archive.

## Usage

``` r
archive_read(
  archive,
  file = 1L,
  mode = "r",
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

An 'archive_read' connection to the file within the archive to be read.

## Examples

``` r
a <- system.file(package = "archive", "extdata", "data.zip")
# Show files in archive
a
#> [1] "/home/runner/work/_temp/Library/archive/extdata/data.zip"

# By default reads the first file in the archive.
read.csv(archive_read(a), nrows = 3)
#>   Sepal.Length Sepal.Width Petal.Length Petal.Width Species
#> 1          5.1         3.5          1.4         0.2  setosa
#> 2          4.9         3.0          1.4         0.2  setosa
#> 3          4.7         3.2          1.3         0.2  setosa

# Can also specify a filename directly
read.csv(archive_read(a, "mtcars.csv"), nrows = 3)
#>    mpg cyl disp  hp drat    wt  qsec vs am gear carb
#> 1 21.0   6  160 110 3.90 2.620 16.46  0  1    4    4
#> 2 21.0   6  160 110 3.90 2.875 17.02  0  1    4    4
#> 3 22.8   4  108  93 3.85 2.320 18.61  1  1    4    1

# Or by position
read.csv(archive_read(a, 3), nrows = 3)
#>   Ozone Solar.R Wind Temp Month Day
#> 1    41     190  7.4   67     5   1
#> 2    36     118  8.0   72     5   2
#> 3    12     149 12.6   74     5   3

# Explicitly specify the format and filter if automatic detection fails.
read.csv(archive_read(a, format = "zip"), nrows = 3)
#>   Sepal.Length Sepal.Width Petal.Length Petal.Width Species
#> 1          5.1         3.5          1.4         0.2  setosa
#> 2          4.9         3.0          1.4         0.2  setosa
#> 3          4.7         3.2          1.3         0.2  setosa
```
