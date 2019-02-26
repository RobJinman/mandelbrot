cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
include(common)

set(WX_CMAKE_ARGS
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
  -DBUILD_SHARED_LIBS=FALSE
)

ExternalProject_Add(libwx
  PREFIX wx_src
  URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.2/wxWidgets-3.1.2.tar.bz2
  CMAKE_ARGS ${WX_CMAKE_ARGS}
  INSTALL_DIR ${PROJECT_SOURCE_DIR}/../vendor/${PLATFORM_NAME}
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

