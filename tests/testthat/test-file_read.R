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
