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
