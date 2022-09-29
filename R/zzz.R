.onLoad <- function(libname, pkgname) {
  lib_path <- list.files(paste0(libname, "/archive/lib/", .Platform$r_arch), pattern="libconnection.*", full.names=TRUE)

print( list.files(paste0(libname, "/archive/lib", .Platform$r_arch) ))

  res <- dyn.load(lib_path)
  rchive_init(res$new_connection$address, res$read_connection$address)
}

.onUnload <- function(libname) {
  lib_path <- list.files(paste0(libname, "/lib/", .Platform$r_arch), pattern="libconnection.*", full.names=TRUE)
  dyn.unload(lib_path)
}
