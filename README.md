
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

``` r
# Write data to zip
w_con <- archive_write("mtcars.7z", "mtcars.csv")
readr::write_csv(mtcars, w_con)

# Read from zip
a <- archive("mtcars.zip")
a
#> # A tibble: 1 × 3
#>         path  size       date
#>        <chr> <dbl>     <dttm>
#> 1 mtcars.csv  1281 1980-01-01

con2 <- archive_read(a, 1)

mt <- readr::read_csv(con2)
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
mt
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

# Write and Read from 7z
readr::write_csv(mt, archive_write("mtcars.7z", "mtcars.csv"))
readr::read_csv(archive_read("mtcars.7z"))
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
file.size(c("mtcars.zip", "mtcars.7z"))
#> [1] 714 677
```
