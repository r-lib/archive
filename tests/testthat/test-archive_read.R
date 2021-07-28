data_file <- system.file(package = "archive", "extdata", "data.zip")

describe("archive_read", {
  it("creates a read only connection", {
    in_con <- file(data_file)
    con <- archive_read(in_con)
    on.exit({close(in_con); close(con)})
    expect_is(con, "connection")
    expect_is(con, "archive_read")

    s <- summary(con)

    expect_true(startsWith(s[["description"]], "archive_read"))
    expect_equal(s[["mode"]], "r")
    expect_equal(s[["text"]], "text")
    expect_equal(s[["opened"]], "closed")
    expect_equal(s[["can read"]], "yes")
    expect_equal(s[["can write"]], "no")
  })
  it("can be read from with a text connection", {
    con <- archive_read(data_file)

    i <- iris
    i$Species <- as.character(i$Species)

    expect_equal(read.csv(con, stringsAsFactors = FALSE), head(i))
  })

  it("can be read from with a binary connection", {
    con <- archive_read(data_file, mode = "rb")

    text <- rawToChar(readBin(con, "raw", n = file.info(data_file)$size))
    close(con)

    i <- iris
    i$Species <- as.character(i$Species)

    expect_equal(read.csv(text = text, stringsAsFactors = FALSE), head(i))
  })

  it("works with readRDS", {
    on.exit(unlink("archive.tar"))

    w_con <- archive_write(archive = "archive.tar", file = "mtcars")
    saveRDS(mtcars, w_con)
    close(w_con)

    r_con <- archive_read(archive = "archive.tar", file = "mtcars")
    out <- readRDS(r_con)
    expect_false(isOpen(r_con))
    close(r_con)

    expect_identical(out, mtcars)
  })
  it("takes options", {
    skip_on_os("windows")
    skip_on_os("solaris")
    skip_if(l10n_info()$`UTF-8` == FALSE)

    con <- archive_read(test_path("cp866.tar.Z.uu"), "\u043f\u0440\u0438\u0432\u0435\u0442", mode = "rb", options = "hdrcharset=CP866")
    on.exit(close(con))

    res <- readBin(con, what = raw(), n = 100)

    res_utf8 <- iconv(list(res), from = "CP866", to = "UTF-8")

    expect_equal(res_utf8, "\u0401\u0404\u0449\u045e\u0445\u0407")
  })
})
