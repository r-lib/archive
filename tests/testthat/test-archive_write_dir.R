describe("archive_write_dir", {
  it("can write a zip file", {
    dir <- normalizePath(tempfile())
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
