# Build against static libraries from rwinlib
if (!file.exists("../windows/libarchive-3.3.3/include")){
  if (getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/rwinlib/libarchive/archive/v3.3.3.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
