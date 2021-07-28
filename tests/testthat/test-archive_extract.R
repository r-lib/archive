data_file <- system.file(package = "archive", "extdata", "data.zip")

describe("archive_extract", {
  it("extracts all files in the archive", {
    a <- archive(data_file)
    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(data_file, d)

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 3)
    expect_equal(file.size(f), a[order(a[["path"]]), ][["size"]])
  })
  it("extracts given files in the archive, indexed by integer", {
    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(data_file, d, files = c(1L, 3L))

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 2)

    a <- archive(data_file)
    expect_equal(sort(file.size(f)), sort(a[a$path %in% a$path[c(1, 3)], ][["size"]]))
  })
  it("extracts given files in the archive, indexed by double", {
    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(data_file, d, files = c(1, 3))

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 2)

    a <- archive(data_file)
    expect_equal(sort(file.size(f)), sort(a[a$path %in% a$path[c(1, 3)], ][["size"]]))
  })
  it("extracts given files in the archive, indexed by name", {
    d <- tempfile()
    on.exit(unlink(d, recursive = TRUE))
    archive_extract(data_file, d, files = c("mtcars.csv", "iris.csv"))

    f <- list.files(d, full.names = TRUE)
    expect_equal(length(f), 2)

    a <- archive(data_file)
    expect_equal(sort(file.size(f)), sort(a[a$path %in% c("mtcars.csv", "iris.csv"), ][["size"]]))
  })
  it("can take options", {
    skip_on_os("windows")
    skip_on_os("solaris")
    skip_if(l10n_info()$`UTF-8` == FALSE)

    out_dir <- tempfile()
    dir.create(out_dir)
    on.exit(unlink(out_dir, recursive = TRUE))

    filename <- "\u043f\u0440\u0438\u0432\u0435\u0442"
    archive_extract(test_path("cp866.tar.Z.uu"), out_dir, filename, options = "hdrcharset=CP866")

    files <- list.files(out_dir)
    Encoding(files) <- "UTF-8"

    expect_equal(files, filename)
  })
})

