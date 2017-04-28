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

    expect_equal(basename(s[["description"]]), "test.zip")
    expect_equal(s[["mode"]], "rb")
    expect_equal(s[["text"]], "binary")
    expect_equal(s[["opened"]], "closed")
    expect_equal(s[["can read"]], "yes")
    expect_equal(s[["can write"]], "no")
  })
  it("can be read from", {
    con <- archive_read("test.zip")
    on.exit(close(con))

    lines <- readLines(con)
    expect_equal(length(lines), 7)
  })
})
