
<!-- README.md is generated from README.Rmd. Please edit that file -->
archive
=======

[![Travis-CI Build Status](https://travis-ci.org/jimhester/archive.svg?branch=master)](https://travis-ci.org/jimhester/archive) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/jimhester/archive?branch=master&svg=true)](https://ci.appveyor.com/project/jimhester/archive)

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
library(readr) # read_csv(), write_csv(), cols()

# Write a single dataset to zip
write_csv(mtcars, archive_write("mtcars.zip", "mtcars.csv"))

# Read the data back, by default the first file is read from the archive.
read_csv(archive_read("mtcars.zip"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1  21.0     6   160   110  3.90 2.620 16.46     0     1     4     4
#> 2  21.0     6   160   110  3.90 2.875 17.02     0     1     4     4
#> 3  22.8     4   108    93  3.85 2.320 18.61     1     1     4     1
#> 4  21.4     6   258   110  3.08 3.215 19.44     1     0     3     1
#> # ... with 28 more rows

# Also supports archiving and compression together
# Write a single dataset to zip
write_csv(mtcars, archive_write("mtcars.tar.gz", "mtcars.csv"))

# Read the data back
read_csv(archive_read("mtcars.tar.gz"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1  21.0     6   160   110  3.90 2.620 16.46     0     1     4     4
#> 2  21.0     6   160   110  3.90 2.875 17.02     0     1     4     4
#> 3  22.8     4   108    93  3.85 2.320 18.61     1     1     4     1
#> 4  21.4     6   258   110  3.08 3.215 19.44     1     0     3     1
#> # ... with 28 more rows

# Archive file sizes
file.size(c("mtcars.zip", "mtcars.tar.gz"))
#> [1] 860 709
```

### Multi file archives

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
#>             path  size                date
#>            <chr> <dbl>              <dttm>
#> 1       iris.csv  3716 2017-04-27 09:28:55
#> 2     mtcars.csv  1281 2017-04-27 09:28:55
#> 3 airquality.csv  2890 2017-04-27 09:28:55

# Read a specific file from the archive
read_csv(archive_read(a, "mtcars.csv"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1  21.0     6   160   110  3.90 2.620 16.46     0     1     4     4
#> 2  21.0     6   160   110  3.90 2.875 17.02     0     1     4     4
#> 3  22.8     4   108    93  3.85 2.320 18.61     1     1     4     1
#> 4  21.4     6   258   110  3.08 3.215 19.44     1     0     3     1
#> # ... with 28 more rows
```

### Regular files (with compression)

`file_write()` returns a connection to filtered by one or more compressions or encodings. `file_read()` reads a compressed file, automatically detecting the compression used.

``` r
# Write bzip2, base 64 encoded data
write_csv(mtcars, file_write("mtcars.bz2", c("uuencode", "bzip2")))

# Read it back
read_csv(file_read("mtcars.bz2"), col_types = cols())
#> # A tibble: 32 × 11
#>     mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>   <dbl> <int> <dbl> <int> <dbl> <dbl> <dbl> <int> <int> <int> <int>
#> 1  21.0     6   160   110  3.90 2.620 16.46     0     1     4     4
#> 2  21.0     6   160   110  3.90 2.875 17.02     0     1     4     4
#> 3  22.8     4   108    93  3.85 2.320 18.61     1     1     4     1
#> 4  21.4     6   258   110  3.08 3.215 19.44     1     0     3     1
#> # ... with 28 more rows
```
