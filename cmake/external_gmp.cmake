cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
include(common)

ExternalProject_Add(
  libGMP
  URL https://gmplib.org/download/gmp/gmp-6.1.2.tar.lz
  PREFIX gmp_src
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/gmp_src/src/libGMP/configure"
                    --prefix "${PROJECT_SOURCE_DIR}/../vendor/${PLATFORM_NAME}"
  BUILD_COMMAND make -j4
  INSTALL_COMMAND make install
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)
