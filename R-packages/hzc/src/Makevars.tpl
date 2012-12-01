## !! Generated by cmake.  Do not edit.
## Makevars for hzc
#
# SRC_DIR          = @SRC_DIR@
# BUILD_DIR        = @BUILD_DIR@
# R_PKG_DIR        = @R_PKG_DIR@
# R_LIBS           = @R_LIBS@

# CXX flags for locating headers and libs
PKG_CPPFLAGS = \
-I@SRC_DIR@ \
-DR_EXTENSION

## Use the R_HOME indirection to support installations of multiple R version
PKG_LIBS = \
`$(R_HOME)/bin/Rscript -e "Rcpp:::LdFlags()"` \
-L@SRC_DIR@/zmq -latp_zmq \
-L@SRC_DIR@/historian -latp_historian \
-L@SRC_DIR@/proto -latp_proto \
-lprotobuf \
-lzmq \
-lboost_system \
-lgflags -lglog





