# Changelog

## archive (development version)

- Fix configure for R-4.6 (stop using CXX11)

## archive 1.1.12

CRAN release: 2025-03-20

- No changes.

## archive 1.1.11

CRAN release: 2025-02-05

- No changes.

## archive 1.1.10

CRAN release: 2024-11-08

- No changes.

## archive 1.1.9

CRAN release: 2024-09-12

- No changes.

## archive 1.1.8

CRAN release: 2024-04-28

- [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md)
  now stop reading the input file once the desired files are extracted,
  instead of always reading to the end of the file
  ([\#85](https://github.com/r-lib/archive/issues/85),
  [@allenluce](https://github.com/allenluce)).

## archive 1.1.7

CRAN release: 2023-12-11

- Fixed an unsafe `printf()` format string
  ([\#95](https://github.com/r-lib/archive/issues/95)).

## archive 1.1.6

CRAN release: 2023-09-18

- Fixed compilation issues on Big-endian platforms
  ([\#84](https://github.com/r-lib/archive/issues/84),
  [@barracuda156](https://github.com/barracuda156)).

## archive 1.1.5

CRAN release: 2022-05-06

- archive now does not produce broken archives on Windows
  ([\#72](https://github.com/r-lib/archive/issues/72),
  [@cielavenir](https://github.com/cielavenir)).

## archive 1.1.4

CRAN release: 2022-01-30

- [`archive_write_dir()`](https://archive.r-lib.org/dev/reference/archive_write_files.md)
  now works with relative paths
  ([\#69](https://github.com/r-lib/archive/issues/69)).

- archive now works properly on UCRT Windows R.

## archive 1.1.3

CRAN release: 2021-11-30

- Gábor Csárdi is now the maintainer.

## archive 1.1.2

CRAN release: 2021-10-25

- Fix UBSAN error related to the progress bar initialization
  ([\#55](https://github.com/r-lib/archive/issues/55))

- Fix unterminated progress bars in
  [`archive_write()`](https://archive.r-lib.org/dev/reference/archive_write.md)
  and friends ([\#60](https://github.com/r-lib/archive/issues/60),
  [@salim-b](https://github.com/salim-b))

## archive 1.1.1

CRAN release: 2021-10-13

- [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md)
  now returns the extracted files (invisibly)
  ([\#50](https://github.com/r-lib/archive/issues/50))

- [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md),
  [`archive_write_files()`](https://archive.r-lib.org/dev/reference/archive_write_files.md)
  and
  [`archive_write_dir()`](https://archive.r-lib.org/dev/reference/archive_write_files.md)
  gain progress bars using the cli package
  ([\#49](https://github.com/r-lib/archive/issues/49))

- Remove uses of deprecated function `glue::collapse()`.

## archive 1.1.0

CRAN release: 2021-08-05

- [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md)
  gains a `strip_components` argument to strip leading pathname
  components if desired
  ([\#27](https://github.com/r-lib/archive/issues/27))

- [`archive()`](https://archive.r-lib.org/dev/reference/archive.md),
  [`archive_read()`](https://archive.r-lib.org/dev/reference/archive_read.md),
  [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md)
  and
  [`file_read()`](https://archive.r-lib.org/dev/reference/file_connections.md)
  now all accept R connections in addition to file paths. This allows
  you to do things like read remote archives with a
  [`url()`](https://rdrr.io/r/base/connections.html) or
  [`curl::curl()`](https://jeroen.r-universe.dev/curl/reference/curl.html)
  connection like you would a file on disk
  ([\#7](https://github.com/r-lib/archive/issues/7))

## archive 1.0.2

CRAN release: 2021-07-16

- skip tests failing on non-UTF-8 systems

## archive 1.0.1

CRAN release: 2021-07-09

- Fix encoding issues with non-UTF-8 linux systems.

## archive 1.0.0

CRAN release: 2021-06-30

- Initial release
