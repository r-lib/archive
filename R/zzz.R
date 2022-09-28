.onLoad <- function(libname, pkgname) {
  lib_path <- list.files(paste0(libname, "/archive/lib"), pattern="libconnection.*", full.names=TRUE)
  res <- dyn.load(lib_path)
  rchive_init(res$new_connection$address, res$read_connection$address)
}

.onUnload <- function(libname) {
  lib_path <- list.files(paste0(libname, "/lib"), pattern="libconnection.*", full.names=TRUE)
  dyn.unload(lib_path)
}
