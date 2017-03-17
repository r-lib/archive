# Credit Jeroen Ooms
# Originally from https://github.com/cran/curl/blob/3897ba5203dee940e2ce40ac23a0d1106da93df6/tools/winlibs.R
# Build against static libraries from rwinlib
if (!file.exists("../windows/libarchive-3.2.2/include")){
  if (getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/rwinlib/libarchive/archive/v3.2.2.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
