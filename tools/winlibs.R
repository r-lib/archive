VERSION <- commandArgs(TRUE)
if(!file.exists(sprintf("../windows/libarchive-%s/include", VERSION))){
  download.file(sprintf("https://github.com/rwinlib/libarchive/archive/v%s.zip", VERSION), "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
