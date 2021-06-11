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

describe("archive_extract", {
  it("extracts all files in the archive", {
    a <- archive(data_file)
    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(a, d)

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 3)
    expect_equal(file.size(f), a[order(a[["path"]]), ][["size"]])
  })
  it("extracts given files in the archive, indexed by number", {
    a <- archive(data_file)

    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(a, d, file = 1)

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 1)
    expect_equal(file.size(f), a[a$path == a$path[[1]], ][["size"]])
  })
  it("extracts given files in the archive, indexed by name", {
    a <- archive(data_file)

    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(a, d, file = "mtcars.csv")

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 1)
    expect_equal(file.size(f), a[a$path == "mtcars.csv", ][["size"]])
  })
})

describe("archive_write", {
  it("creates a writable connection", {
    out_con <- archive_write("mtcars.zip", "mtcars.csv")
    on.exit({
      unlink("mtcars.zip")
    })

    expect_is(out_con, "connection")
    expect_is(out_con, "archive_write")

    write.csv(file = out_con, mtcars)

    in_con <- unz("mtcars.zip", "mtcars.csv")
    data <- read.csv(in_con, row.names = 1)

    expect_equal(data, mtcars)
  })

  it("works with all supported formats", {
    extensions <-
      c("7z",
        "cpio",
        "iso",
        #"mtree", mtree is a textual description of a file hierarchy, rather than a normal archive format.
        "tar",
        "tgz",
        "taz",
        "tar.gz",
        "tbz",
        "tbz2",
        "tz2",
        "tar.bz2",
        "tlz",
        "tar.lzma",
        "txz",
        "tar.xz",
        #"tzo",
        "taZ",
        "tZ",
        #"warc",
        "jar",
        "zip")

    if (libarchive_version() >= "3.3.3") {
      extensions <- c(extensions, "tar.zst")
    }

    test_extension <- function(ext) {
      filename <- paste0("mtcars", ".", ext)
      on.exit(unlink(filename))

      expect_error(write.csv(mtcars, archive_write(filename, "mtcars.csv")), NA, info = ext)
      expect_equal(read.csv(archive_read(filename, "mtcars.csv"), row.names = 1), mtcars)
    }
    for (ext in extensions) {
      test_extension(ext)
    }
  })

  it("can take options", {
    f <- tempfile(fileext = ".tar.gz")
    f2 <- tempfile(fileext = ".tar.gz")
    on.exit(unlink(c(f, f2)))

    write.csv(mtcars,
      archive_write(f, "mtcars.csv", options = "compression-level=0")
    )

    write.csv(mtcars,
      archive_write(f2, "mtcars.csv", options = "compression-level=9")
    )

    expect_gt(file.size(f), file.size(f2))
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

describe("archive_write_files", {
  it("can write a zip file", {
    files <- c(mtcars = "mtcars.csv", iris = "iris.csv")
    archive <- tempfile(fileext = ".zip")

    on.exit(unlink(c(files, archive)))

    write.csv(mtcars, files[["mtcars"]])
    write.csv(iris, files[["iris"]])

    archive_write_files(archive, files)

    expect_equal(
      read.csv(unz(archive, files[["mtcars"]]), row.names = 1),
      mtcars)

    expect_equal(
      read.csv(unz(archive, files[["iris"]]), row.names = 1, stringsAsFactors = TRUE),
      iris)
    })

  it("takes options", {
    files <- c(mtcars = tempfile(fileext = ".csv"), iris = tempfile(fileext = ".csv"))
    archive <- tempfile(fileext = ".zip")
    archive2 <- tempfile(fileext = ".zip")
    on.exit(unlink(c(files, archive)))

    write.csv(mtcars, files[["mtcars"]])
    write.csv(iris, files[["iris"]])

    archive_write_files(archive, files, options = "compression-level=0")

    archive_write_files(archive2, files, options = "compression-level=9")

    expect_gt(file.size(archive), file.size(archive2))
  })
})

describe("archive_write_dir", {
  it("can write a zip file", {
    dir <- tempfile()
    dir.create(dir)
    files <- c(mtcars = "mtcars.csv", iris = "iris.csv")

    write.csv(mtcars, file.path(dir, files[["mtcars"]]))
    write.csv(iris, file.path(dir, files[["iris"]]))

    archive_write_dir("data.zip", dir)
    on.exit(unlink(c(files, "data.zip")))

    expect_equal(
      read.csv(unz("data.zip", files[["mtcars"]]), row.names = 1),
      mtcars)

    expect_equal(
      read.csv(unz("data.zip", files[["iris"]]), row.names = 1, stringsAsFactors = TRUE),
      iris)
    })
  })
