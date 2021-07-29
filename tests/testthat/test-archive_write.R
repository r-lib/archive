describe("archive_write", {
  it("creates a writable connection", {
    a <- tempfile(fileext = ".zip")
    out_con <- archive_write(a, "mtcars.csv")
    on.exit({
      unlink(a)
    })

    expect_is(out_con, "connection")
    expect_is(out_con, "archive_write")

    write.csv(file = out_con, mtcars)

    in_con <- unz(a, "mtcars.csv")
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
      filename <- tempfile(fileext = paste0(".", ext))
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
  it("zip writer can take options", {
    f <- tempfile(fileext = ".zip")
    f2 <- tempfile(fileext = ".zip")
    on.exit(unlink(c(f, f2)))

    write.csv(mtcars,
      archive_write(f, "mtcars.csv", options = "compression-level=1")
    )

    write.csv(mtcars,
      archive_write(f2, "mtcars.csv", options = "compression-level=9")
    )

    expect_gt(file.size(f), file.size(f2))
  })

  it("can translate character sets with a cpio archive", {
    skip_on_os("windows")
    skip_on_os("solaris")
    skip_on_os("mac") # for some unknown reason this test fails on macOS

    f <- tempfile(fileext = ".cpio")

    filename <- "\u0401\u0404\u0449\u045e\u0445\u0407"
    write.csv(mtcars,
      archive_write(f, filename, options = "hdrcharset=CP866")
    )
    a <- archive(f, options = "hdrcharset=CP866")

    expect_equal(a$path, filename)
  })

  it("can translate character sets with a zip archive", {
    skip_on_os("windows")
    skip_on_os("solaris")
    skip_on_os("mac") # for some unknown reason this test fails on macOS

    f <- tempfile(fileext = ".zip")

    filename <- "\u0401\u0404\u0449\u045e\u0445\u0407"
    write.csv(mtcars,
      archive_write(f, filename, options = "hdrcharset=CP866")
    )
    a <- archive(f, options = "hdrcharset=CP866")

    expect_equal(a$path, filename)
  })
})

