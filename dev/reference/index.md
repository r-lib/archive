# Package index

## Read and write archives using R connections.

These functions deal with archive formats such as zip, 7zip, rar and tar
and return connection objects which can be used by many R input / output
functions.

- [`archive()`](https://archive.r-lib.org/dev/reference/archive.md) :
  Construct a new archive
- [`archive_read()`](https://archive.r-lib.org/dev/reference/archive_read.md)
  : Create a readable connection to a file in an archive.
- [`archive_write()`](https://archive.r-lib.org/dev/reference/archive_write.md)
  : Create a writable connection to a file in an archive.

## Extract files from archives and write existing files to archives.

These functions create archives from a set of existing files or extract
some or all files from an archive to disk.

- [`archive_extract()`](https://archive.r-lib.org/dev/reference/archive_extract.md)
  : Extract contents of an archive to a directory
- [`archive_write_dir()`](https://archive.r-lib.org/dev/reference/archive_write_files.md)
  [`archive_write_files()`](https://archive.r-lib.org/dev/reference/archive_write_files.md)
  : Add files to a new archive

## Read and Write files using R connections.

These functions write or read a file filtered by one or more compression
algorithms or encoding filters supported by libarchive, such as gzip,
bzip2 and xz and return an R connection to that file.

- [`file_read()`](https://archive.r-lib.org/dev/reference/file_connections.md)
  [`file_write()`](https://archive.r-lib.org/dev/reference/file_connections.md)
  : Construct a connections for (possibly compressed) files.
