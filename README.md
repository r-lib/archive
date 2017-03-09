
<!-- README.md is generated from README.Rmd. Please edit that file -->
archive
=======

[![Travis-CI Build Status](https://travis-ci.org/jimhester/archive.svg?branch=master)](https://travis-ci.org/jimhester/archive)

R bindings to libarchive <http://www.libarchive.org>, providing support for many file archives, including tar, ZIP, 7-zip, RAR, CAB including those compressed by gzip, bzip2, compress, lzma, xz, and others.

Installation
------------

You can install archive from github with:

``` r
# install.packages("devtools")
devtools::install_github("jimhester/archive")
```

Example
-------

### Single file archives

``` r
# Write a single dataset to zip
readr::write_csv(mtcars, archive_write("mtcars.zip", "mtcars.csv"))

# Read the data back, by default the first file is read from the archive.
readr::read_csv(archive_read("mtcars.zip"))
#> Parsed with column specification:
#> cols(
#>   mpg = col_double(),
#>   cyl = col_integer(),
#>   disp = col_double(),
#>   hp = col_integer(),
#>   drat = col_double(),
#>   wt = col_double(),
#>   qsec = col_double(),
#>   vs = col_integer(),
#>   am = col_integer(),
#>   gear = col_integer(),
#>   carb = col_integer()
#> )
#> # A tibble: 32 × 11
#>      mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>    <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1   21.0     6 160.0   110  3.90 2.620 16.46     0     1     4     4
#> 2   21.0     6 160.0   110  3.90 2.875 17.02     0     1     4     4
#> 3   22.8     4 108.0    93  3.85 2.320 18.61     1     1     4     1
#> 4   21.4     6 258.0   110  3.08 3.215 19.44     1     0     3     1
#> 5   18.7     8 360.0   175  3.15 3.440 17.02     0     0     3     2
#> 6   18.1     6 225.0   105  2.76 3.460 20.22     1     0     3     1
#> 7   14.3     8 360.0   245  3.21 3.570 15.84     0     0     3     4
#> 8   24.4     4 146.7    62  3.69 3.190 20.00     1     0     4     2
#> 9   22.8     4 140.8    95  3.92 3.150 22.90     1     0     4     2
#> 10  19.2     6 167.6   123  3.92 3.440 18.30     1     0     4     4
#> # ... with 22 more rows

# Also supports archiving and compression together
# Write a single dataset to zip
readr::write_csv(mtcars, archive_write("mtcars.tar.gz", "mtcars.csv"))

# Read the data back
readr::read_csv(archive_read("mtcars.tar.gz"))
#> Parsed with column specification:
#> cols(
#>   mpg = col_double(),
#>   cyl = col_integer(),
#>   disp = col_double(),
#>   hp = col_integer(),
#>   drat = col_double(),
#>   wt = col_double(),
#>   qsec = col_double(),
#>   vs = col_integer(),
#>   am = col_integer(),
#>   gear = col_integer(),
#>   carb = col_integer()
#> )
#> # A tibble: 32 × 11
#>      mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>    <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1   21.0     6 160.0   110  3.90 2.620 16.46     0     1     4     4
#> 2   21.0     6 160.0   110  3.90 2.875 17.02     0     1     4     4
#> 3   22.8     4 108.0    93  3.85 2.320 18.61     1     1     4     1
#> 4   21.4     6 258.0   110  3.08 3.215 19.44     1     0     3     1
#> 5   18.7     8 360.0   175  3.15 3.440 17.02     0     0     3     2
#> 6   18.1     6 225.0   105  2.76 3.460 20.22     1     0     3     1
#> 7   14.3     8 360.0   245  3.21 3.570 15.84     0     0     3     4
#> 8   24.4     4 146.7    62  3.69 3.190 20.00     1     0     4     2
#> 9   22.8     4 140.8    95  3.92 3.150 22.90     1     0     4     2
#> 10  19.2     6 167.6   123  3.92 3.440 18.30     1     0     4     4
#> # ... with 22 more rows

# Archive file sizes
file.size(c("mtcars.zip", "mtcars.tar.gz"))
#> [1] 738 640
```

### Multi file archives

``` r
# Write a few files to the temp directory
readr::write_csv(iris, "iris.csv")
readr::write_csv(mtcars, "mtcars.csv")
readr::write_csv(airquality, "airquality.csv")

# Add them to a new archive
archive_write_files("data.tar.xz", c("iris.csv", "mtcars.csv", "airquality.csv"))

# View archive contents
a <- archive("data.tar.xz")
a
#> # A tibble: 3 × 3
#>             path  size                date
#>            <chr> <dbl>              <dttm>
#> 1       iris.csv  3716 2017-03-09 17:35:42
#> 2     mtcars.csv  1281 2017-03-09 17:35:42
#> 3 airquality.csv  2890 2017-03-09 17:35:42

# Read a specific file from the archive
readr::read_csv(archive_read(a, "mtcars.csv"))
#> Parsed with column specification:
#> cols(
#>   mpg = col_double(),
#>   cyl = col_integer(),
#>   disp = col_double(),
#>   hp = col_integer(),
#>   drat = col_double(),
#>   wt = col_double(),
#>   qsec = col_double(),
#>   vs = col_integer(),
#>   am = col_integer(),
#>   gear = col_integer(),
#>   carb = col_integer()
#> )
#> # A tibble: 32 × 11
#>      mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>    <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1   21.0     6 160.0   110  3.90 2.620 16.46     0     1     4     4
#> 2   21.0     6 160.0   110  3.90 2.875 17.02     0     1     4     4
#> 3   22.8     4 108.0    93  3.85 2.320 18.61     1     1     4     1
#> 4   21.4     6 258.0   110  3.08 3.215 19.44     1     0     3     1
#> 5   18.7     8 360.0   175  3.15 3.440 17.02     0     0     3     2
#> 6   18.1     6 225.0   105  2.76 3.460 20.22     1     0     3     1
#> 7   14.3     8 360.0   245  3.21 3.570 15.84     0     0     3     4
#> 8   24.4     4 146.7    62  3.69 3.190 20.00     1     0     4     2
#> 9   22.8     4 140.8    95  3.92 3.150 22.90     1     0     4     2
#> 10  19.2     6 167.6   123  3.92 3.440 18.30     1     0     4     4
#> # ... with 22 more rows
```

### Regular files (with compression)

`file_write()` returns a connection to filtered by one or more compressions or encodings.

``` r
# Write bzip2, base 64 encoded data
readr::write_csv(mtcars, file_write("mtcars.bz2", c("b64encode", "bzip2")))
```
