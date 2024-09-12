# archive 1.1.9

* No changes.

# archive 1.1.8

* `archive_extract()` now stop reading the input file once the desired
  files are extracted, instead of always reading to the end of the file
  (#85, @allenluce).

# archive 1.1.7

* Fixed an unsafe `printf()` format string (#95).

# archive 1.1.6

* Fixed compilation issues on Big-endian platforms (#84, @barracuda156).

# archive 1.1.5

* archive now does not produce broken archives on Windows (#72, @cielavenir).

# archive 1.1.4

* `archive_write_dir()` now works with relative paths (#69).

* archive now works properly on UCRT Windows R.

# archive 1.1.3

* Gábor Csárdi is now the maintainer.

# archive 1.1.2

* Fix UBSAN error related to the progress bar initialization (#55)

* Fix unterminated progress bars in `archive_write()` and friends (#60, @salim-b)

# archive 1.1.1

* `archive_extract()` now returns the extracted files (invisibly) (#50)

* `archive_extract()`, `archive_write_files()` and `archive_write_dir()` gain progress bars using the cli package (#49)

* Remove uses of deprecated function `glue::collapse()`.

# archive 1.1.0

* `archive_extract()` gains a `strip_components` argument to strip leading pathname components if desired (#27)

* `archive()`, `archive_read()`, `archive_extract()` and `file_read()` now all accept R connections in addition to file paths.
  This allows you to do things like read remote archives with a `url()` or `curl::curl()` connection like you would a file on disk (#7)

# archive 1.0.2

* skip tests failing on non-UTF-8 systems

# archive 1.0.1

* Fix encoding issues with non-UTF-8 linux systems.

# archive 1.0.0

* Initial release
