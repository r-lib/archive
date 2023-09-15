
<!-- README.md is generated from README.Rmd. Please edit that file -->

# archive

<!-- badges: start -->

[![R-CMD-check](https://github.com/r-lib/archive/workflows/R-CMD-check/badge.svg)](https://github.com/r-lib/archive/actions)
[![Coverage
Status](https://img.shields.io/codecov/c/github/r-lib/archive/main)](https://codecov.io/github/r-lib/archive?branch=main)
[![CRAN
status](https://www.r-pkg.org/badges/version/archive)](https://CRAN.R-project.org/package=archive)
<!-- badges: end -->

R bindings to libarchive <http://www.libarchive.org>. Supports many
archives formats, including tar, ZIP, 7-zip, RAR, CAB. Also supports
many filters such as gzip, bzip2, compress, lzma, xz and uuencoded
files, among others.

archive provides interfaces to read and write connections into archives,
as well as efficiently reading and writing archives directly to disk.

## Installation

You can install archive from CRAN with:

``` r
# install.packages("archive")
```

## Example

### Single file archives

Use `archive_read()` and `archive_write()` to read and write single
files to an archive. These files return connections, which can be passed
to any R interface which can take a connection. Most base R file system
functions use connections, as well as some packages like
[readr](https://readr.tidyverse.org/).

``` r
library(readr) # read_csv(), write_csv(), cols()

# Write a single dataset to zip
write_csv(mtcars, archive_write("mtcars.zip", "mtcars.csv"))

# Read the data back, by default the first file is read from the archive.
read_csv(archive_read("mtcars.zip"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1  21       6   160   110  3.9   2.62  16.5     0     1     4     4
#> 2  21       6   160   110  3.9   2.88  17.0     0     1     4     4
#> 3  22.8     4   108    93  3.85  2.32  18.6     1     1     4     1
#> 4  21.4     6   258   110  3.08  3.22  19.4     1     0     3     1
#> # ℹ 28 more rows

# Also supports things like archiving and compression together
# Write a single dataset to (gzip compressed) tar
write_csv(mtcars, archive_write("mtcars.tar.gz", "mtcars.csv", options = "compression-level=9"))

# Read the data back
read_csv(archive_read("mtcars.tar.gz"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1  21       6   160   110  3.9   2.62  16.5     0     1     4     4
#> 2  21       6   160   110  3.9   2.88  17.0     0     1     4     4
#> 3  22.8     4   108    93  3.85  2.32  18.6     1     1     4     1
#> 4  21.4     6   258   110  3.08  3.22  19.4     1     0     3     1
#> # ℹ 28 more rows

# Archive file sizes
file.size(c("mtcars.zip", "mtcars.tar.gz"))
#> [1] 742 648
```

### Multi file archives

`archive_write_files()` is used to create a new archive from multiple
files on disk. `archive_extract()` is used to extract an archive (with
one or more files) into a local folder.

``` r
# Write a few files to the temp directory
write_csv(iris, "iris.csv")
write_csv(mtcars, "mtcars.csv")
write_csv(airquality, "airquality.csv")

# Add them to a new archive
archive_write_files("data.tar.xz", c("iris.csv", "mtcars.csv", "airquality.csv"))

# View archive contents
a <- archive("data.tar.xz")
a
#> # A tibble: 3 × 3
#>   path            size date               
#>   <chr>          <int> <dttm>             
#> 1 iris.csv        3716 2023-07-23 00:53:57
#> 2 mtcars.csv      1281 2023-07-23 00:53:57
#> 3 airquality.csv  2890 2023-07-23 00:53:57

# By default `archive_read()` will read the first file from a multi-file archive.
read_csv(archive_read("data.tar.xz"), col_types = cols())
#> # A tibble: 150 × 5
#>   Sepal.Length Sepal.Width Petal.Length Petal.Width Species
#>          <dbl>       <dbl>        <dbl>       <dbl> <chr>  
#> 1          5.1         3.5          1.4         0.2 setosa 
#> 2          4.9         3            1.4         0.2 setosa 
#> 3          4.7         3.2          1.3         0.2 setosa 
#> 4          4.6         3.1          1.5         0.2 setosa 
#> # ℹ 146 more rows

# Use a number to read a different file
read_csv(archive_read("data.tar.xz", file = 2), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1  21       6   160   110  3.9   2.62  16.5     0     1     4     4
#> 2  21       6   160   110  3.9   2.88  17.0     0     1     4     4
#> 3  22.8     4   108    93  3.85  2.32  18.6     1     1     4     1
#> 4  21.4     6   258   110  3.08  3.22  19.4     1     0     3     1
#> # ℹ 28 more rows

# Or a filename to read a specific file
read_csv(archive_read("data.tar.xz", file = "mtcars.csv"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1  21       6   160   110  3.9   2.62  16.5     0     1     4     4
#> 2  21       6   160   110  3.9   2.88  17.0     0     1     4     4
#> 3  22.8     4   108    93  3.85  2.32  18.6     1     1     4     1
#> 4  21.4     6   258   110  3.08  3.22  19.4     1     0     3     1
#> # ℹ 28 more rows

# # Use `archive_extract()` to extract all the files to a local folder.
d <- tempfile()
archive_extract("data.tar.xz", dir = d)
print(list.files(d))
#> [1] "airquality.csv" "iris.csv"       "mtcars.csv"
unlink(d)

# Or combine with `fs::path_filter()` to extract only a subset of the files.
if(rlang::is_installed("fs")) {
  d <- tempfile()
  archive_extract(
    "data.tar.xz", 
    dir = d,
    files = fs::path_filter(
      archive("data.tar.xz")$path,
      glob="*s.csv"
    )
  )
  print(list.files(d))
  unlink(d)
}
#> [1] "iris.csv"   "mtcars.csv"
```

### Regular files (with compression)

`file_write()` returns a connection to filtered by one or more
compressions or encodings. `file_read()` reads a compressed file,
automatically detecting the compression used.

``` r
# Write bzip2, uuencoded data
write_csv(mtcars, file_write("mtcars.bz2", filter = c("uuencode", "bzip2")))

# Read it back, the formats are automatically detected
read_csv(file_read("mtcars.bz2"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1  21       6   160   110  3.9   2.62  16.5     0     1     4     4
#> 2  21       6   160   110  3.9   2.88  17.0     0     1     4     4
#> 3  22.8     4   108    93  3.85  2.32  18.6     1     1     4     1
#> 4  21.4     6   258   110  3.08  3.22  19.4     1     0     3     1
#> # ℹ 28 more rows
```
