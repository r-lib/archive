context("archive")

data_file <- system.file(package = "archive", "extdata", "data.zip")

describe("archive", {
  it("reads simple zip files", {
    a <- archive(data_file)
    expect_equal(NROW(a), 3L)
    expect_equal(a[["path"]], c("iris.csv", "mtcars.csv", "airquality.csv"))
    expect_equal(a[["size"]], c(192, 274, 142))
  })
})

describe("file_read", {
  it("can read a gzip file", {
    write.csv(mtcars,
      gzfile("test.gz"))

    on.exit(unlink("test.gz"))

    expect_equal(
      read.csv(file_read("test.gz"), row.names = 1),
      mtcars)
  })

  it("can read a xz file", {
    write.csv(mtcars,
      xzfile("test.xz"))

    on.exit(unlink("test.xz"))

    expect_equal(
      read.csv(file_read("test.xz"), row.names = 1),
      mtcars)
  })
})

if (libarchive_version() >= "3.2.0") {
  describe("file_write", {
    it("can write a gzip file", {
      write.csv(mtcars,
        file_write("test.gz"))
      on.exit(unlink("test.gz"))

      expect_equal(
        read.csv(gzfile("test.gz"), row.names = 1),
        mtcars)
    })

    it("can take options", {
      f <- tempfile(fileext = ".gz")
      f2 <- tempfile(fileext = ".gz")
      on.exit(unlink(c(f, f2)))

      write.csv(mtcars,
        file_write(f, options = "compression-level=0")
      )

      write.csv(mtcars,
        file_write(f2, options = "compression-level=9")
      )

      expect_gt(file.size(f), file.size(f2))
    })

    it("can write a xz file", {
      write.csv(mtcars,
        file_write("test.xz"))
      on.exit(unlink("test.xz"))

      expect_equal(
        read.csv(xzfile("test.xz"), row.names = 1),
        mtcars)
    })

    it("works with multiple filters", {
      filename <- "mtcars.bz2.uu"
      write.csv(mtcars, file_write(filename))
      on.exit(unlink(filename))

      expect_equal(read.csv(file_read(filename), row.names = 1), mtcars)
    })

    it("works with all supported formats", {

      extensions <- c(
        "Z",
        "bz2",
        "gz",
        "lz",
        "lz4",
        "lzo",
        "lzma",
        "uu",
        "xz")

      f <- "mtcars.csv"
      test_extension <- function(ext) {
        filename <- paste0(f, ".", ext)
        on.exit(unlink(filename))

        expect_error(write.csv(mtcars, file_write(filename)), NA, info = ext)
        expect_equal(read.csv(file_read(filename), row.names = 1), mtcars)
      }
      for (ext in extensions) {
        test_extension(ext)
      }

    })
  })
}
