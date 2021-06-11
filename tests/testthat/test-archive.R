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
