
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
a <- archive("~/data/trip_fare.7z")
a
#> # A tibble: 12 × 3
#>                path       size                date
#>               <chr>      <dbl>              <dttm>
#> 1   trip_fare_1.csv 1681610043 2013-08-25 21:04:08
#> 2  trip_fare_10.csv 1712562557 2014-01-15 01:49:22
#> 3  trip_fare_11.csv 1641999933 2014-01-15 02:28:06
#> 4  trip_fare_12.csv 1593879813 2014-01-15 03:05:04
#> 5   trip_fare_2.csv 1593003695 2013-08-25 21:36:04
#> 6   trip_fare_3.csv 1794836351 2013-08-25 22:15:36
#> 7   trip_fare_4.csv 1721514415 2013-08-25 22:50:08
#> 8   trip_fare_5.csv 1743431041 2013-08-25 23:24:16
#> 9   trip_fare_6.csv 1641402184 2013-08-25 23:55:34
#> 10  trip_fare_7.csv 1576268209 2014-01-14 23:55:36
#> 11  trip_fare_8.csv 1436875197 2014-01-15 00:30:56
#> 12  trip_fare_9.csv 1610146825 2014-01-15 01:09:06

con <- archive_con(a, 5)

readr::read_csv(con)
#> Parsed with column specification:
#> cols(
#>   medallion = col_character(),
#>   hack_license = col_character(),
#>   vendor_id = col_character(),
#>   pickup_datetime = col_datetime(format = ""),
#>   payment_type = col_character(),
#>   fare_amount = col_double(),
#>   surcharge = col_double(),
#>   mta_tax = col_double(),
#>   tip_amount = col_double(),
#>   tolls_amount = col_double(),
#>   total_amount = col_double()
#> )
#> # A tibble: 13,990,176 × 11
#>                           medallion                     hack_license
#>                               <chr>                            <chr>
#> 1  1B5C0970F2AE8CFFBA8AE4584BEAED29 D961332334524990D1BBD462E2EFB8A4
#> 2  B42249AE16E2B8E556F1CB1F940D6FB4 D4BB308D1F3FCB3434D9DB282CDC93D7
#> 3  890699222C47C09FBC898758CEC69762 6318C3AEC02248928C3345B5805EB905
#> 4  74B7D835C2CD98606D5256DA8A38E045 D5E278C918256D1F97680A1F04D290E0
#> 5  4003B8478418FEC5D761E2F37602769B 0B766F1054A5C16D86BC023858BD8143
#> 6  D72DF7B12201912BFDBB93081EF04C96 AFD828EEF790A2485BBB0B568A8BE22E
#> 7  FA5337E245DE9B2E124AFA735B41B4DF 6E6B7D73303D5AE3808A9ABF4D3FF65B
#> 8  BE63343BAD5CD6F99EC435812E332445 88CF6CAA78CE8B2000FBCF39DFAB3E69
#> 9  58598FD2F8811C4F52A264D15D6FECAF F6F9DF7755F186C183EEBB5B03DBA23C
#> 10 FE13CDF111CE4A3811F3E5D942BFD3B9 BAC511461590FAF7D7BB4C8F29C31AC5
#> # ... with 13,990,166 more rows, and 9 more variables: vendor_id <chr>,
#> #   pickup_datetime <dttm>, payment_type <chr>, fare_amount <dbl>,
#> #   surcharge <dbl>, mta_tax <dbl>, tip_amount <dbl>, tolls_amount <dbl>,
#> #   total_amount <dbl>
```
