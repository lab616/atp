## !! Generated by cmake.  Do not edit.
## Makevars for raptor
#
# IBCLIENT_VERSION = @IBCLIENT_VERSION@
# IBAPI_VERSION    = @IBAPI_VERSION@
# SRC_DIR          = @SRC_DIR@
# BUILD_DIR        = @BUILD_DIR@

# IBAPIConnector lib when built by cmake without 'make install' lives
# in the src/ib directory.

# CXX flags for locating headers and libs
PKG_CPPFLAGS = \
-I@SRC_DIR@ \
-I@IBAPI_ROOT@ -I@IBAPI_ROOT@/Shared -I@SRC_DIR@/ib/@IBAPI_IMPL_DIR@ \
-DIB_USE_STD_STRING

## Use the R_HOME indirection to support installations of multiple R version
PKG_LIBS = \
`$(R_HOME)/bin/Rscript -e "Rcpp:::LdFlags()"` \
-L@BUILD_DIR@/third_party/lib -lleveldb \
-L@SRC_DIR@/ib -lIBAPIConnector \
-L@SRC_DIR@/varz -lvarz_components \
-L@SRC_DIR@/zmq -lzmq_components \
-L@SRC_DIR@/historian -lhistorian_components \
-L@SRC_DIR@/proto -lproto_components \
-lprotobuf \
-lprotobuf-lite \
-lzmq \
-lQuantLib \
-lquickfix \
-lgflags -lglog \





