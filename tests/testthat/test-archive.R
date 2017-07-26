context("archive")

describe("archive", {
  it("reads simple zip files", {
    a <- archive("test.zip")
    expect_equal(NROW(a), 3L)
    expect_equal(a[["path"]], c("iris.csv", "mtcars.csv", "airquality.csv"))
    expect_equal(a[["size"]], c(192, 274, 142))
  })
})

describe("archive_extract", {
  it("creates a read only connection", {
    a <- archive("test.zip")
    d <- tempfile()
    dir.create(d)
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(a, d)

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 3)
    expect_equal(file.size(f), a[order(a[["path"]]), ][["size"]])
  })
})

describe("archive_read", {
  it("creates a read only connection", {
    con <- archive_read("test.zip")
    on.exit(close(con))
    expect_is(con, "connection")
    expect_is(con, "archive_read")

    s <- summary(con)

    expect_equal(basename(s[["description"]]), "test.zip[iris.csv]")
    expect_equal(s[["mode"]], "r")
    expect_equal(s[["text"]], "text")
    expect_equal(s[["opened"]], "closed")
    expect_equal(s[["can read"]], "yes")
    expect_equal(s[["can write"]], "no")
  })
  it("can be read from", {
    con <- archive_read("test.zip")

    i <- iris
    i$Species <- as.character(i$Species)

    expect_equal(read.csv(con, stringsAsFactors = FALSE), head(i))
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
        "tzo",
        "taZ",
        "tZ",
        "warc",
        "jar",
        "zip")

    test_extension <- function(ext) {
      filename <- paste0("mtcars", ".", ext)
      on.exit(unlink(filename))

      write.csv(mtcars, archive_write(filename, "mtcars.csv"))
      expect_equal(read.csv(archive_read(filename, "mtcars.csv"), row.names = 1), mtcars)
    }
    for (ext in extensions) {
      test_extension(ext)
    }
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

        write.csv(mtcars, file_write(filename))
        expect_equal(read.csv(file_read(filename), row.names = 1), mtcars)
      }
      for (ext in extensions) {
        test_extension(ext)
      }

    })
  })

  describe("archive_write_files", {
    it("can write a zip file", {
      files <- c(mtcars = "mtcars.csv", iris = "iris.csv")
      write.csv(mtcars, files[["mtcars"]])
      write.csv(iris, files[["iris"]])

      archive_write_files("data.zip", files)
      on.exit(unlink(c(files, "data.zip")))

      expect_equal(
        read.csv(unz("data.zip", files[["mtcars"]]), row.names = 1),
        mtcars)

      expect_equal(
        read.csv(unz("data.zip", files[["iris"]]), row.names = 1),
        iris)
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
        read.csv(unz("data.zip", files[["iris"]]), row.names = 1),
        iris)
    })
  })
}
