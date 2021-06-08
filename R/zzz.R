.onLoad <- function(libname, pkgname) {
  lib_path <- system.file("lib", .Platform$r_arch, paste0("libconnection", .Platform$dynlib.ext), package = "archive")
  res <- dyn.load(lib_path)

  rchive_init(res$new_connection$address)
}

.onUnload <- function(libname) {
  lib_path <- system.file("lib", .Platform$r_arch, paste0("libconnection", .Platform$dynlib.ext), package = "archive")
  dyn.unload(lib_path)
}
