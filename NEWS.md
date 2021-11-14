# archive (development version)

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
