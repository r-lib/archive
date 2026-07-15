context("archive")

data_file <- system.file(package = "archive", "extdata", "data.zip")

describe("archive", {
  it("reads simple zip files", {
    a <- archive(data_file)
    expect_equal(NROW(a), 3L)
    expect_equal(a[["path"]], c("iris.csv", "mtcars.csv", "airquality.csv"))
    expect_equal(a[["size"]], c(192, 274, 142))
  })
  it("takes options", {
    skip_on_os("windows")
    skip_on_os("solaris")
    skip_if_no_encoding("CP866")
    skip_if(l10n_info()$`UTF-8` == FALSE)

    a <- archive(test_path("cp866.tar.Z.uu"), "hdrcharset=CP866")
    expect_equal(a$path, c("\u041f\u0420\u0418\u0412\u0415\u0422", "\u043f\u0440\u0438\u0432\u0435\u0442"))
  })
  it("works with tar.gz files", {
    # `file()` will implicitly change to a gzfile connection unless `mode =
    # "rb"`, and gzfile cannot seek properly, so we need to ensure we open as
    # a normal file.
    a <- archive(test_path("mtcars.tar.gz"))
    expect_equal(a$path, "mtcars.csv")
  })
})

describe("libarchive_zlib_version", {
  it("handles simple versions", {
    fake(libarchive_zlib_version, "libarchive_zlib_version_", "1.3.1")
    expect_equal(libarchive_zlib_version(), package_version("1.3.1"))
  })
  it("removes non-numeric suffix", {
    fake(libarchive_zlib_version, "libarchive_zlib_version_", "1.3.1.zlib-ng")
    expect_equal(libarchive_zlib_version(), package_version("1.3.1"))
  })
  it("handles nonsensical versions", {
    fake(libarchive_zlib_version, "libarchive_zlib_version_", "1")
    expect_equal(libarchive_zlib_version(), package_version("0.0.0"))
    fake(libarchive_zlib_version, "libarchive_zlib_version_", "1.foobar")
    expect_equal(libarchive_zlib_version(), package_version("0.0.0"))
    fake(libarchive_zlib_version, "libarchive_zlib_version_", "not-really-good")
    expect_equal(libarchive_zlib_version(), package_version("0.0.0"))
  })
})
