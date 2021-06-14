data_file <- system.file(package = "archive", "extdata", "data.zip")

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
  it("can take options", {
    out_dir <- tempfile()
    dir.create(out_dir)
    on.exit(unlink(out_dir, recursive = TRUE))

    filename <- "\u043f\u0440\u0438\u0432\u0435\u0442"
    archive_extract(test_path("cp866.tar.Z.uu"), out_dir, filename, options = "hdrcharset=CP866")
    file.exists(file.path(out_dir, filename))
  })
})

