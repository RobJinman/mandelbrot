cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
include(common)

set(
  GLEW_CMAKE_ARGS
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
  -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
  -DBUILD_SHARED_LIBS=FALSE
)

ExternalProject_Add(
  libglew
  URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz
  PREFIX glew_src
  CMAKE_ARGS ${GLEW_CMAKE_ARGS}
  SOURCE_SUBDIR build/cmake
  INSTALL_DIR ${PROJECT_SOURCE_DIR}/../vendor/${PLATFORM_NAME}
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

