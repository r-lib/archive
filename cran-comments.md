## Test environments
* local OS X install, R 3.4.0
* ubuntu 12.04 (on travis-ci), R 3.4.0
* win-builder (devel and release)
* rhub

## R CMD check results

0 errors | 0 warnings | 2 notes

    * This is a new release.

    * checking compiled code ... NOTE
    File ‘archive/libs/archive.so’:
      Found non-API call to R: ‘R_new_custom_connection’

    Compiled code should not call non-API entry points in R.

This package uses the publicly released API for creating custom connections in
packages.

The package will fail to install on systems without libarchive installed.
Common package libarchive is distributed with are included in the
SystemRequirements field of the DESCRIPTION file.

## Reverse dependencies

This is a new release, so there are no reverse dependencies.
