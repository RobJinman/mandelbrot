cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mandelbrot CXX)

set(Mandelbrot_VERSION_MAJOR 0)
set(Mandelbrot_VERSION_MINOR 1)

set(PLATFORM_LINUX 0)
set(PLATFORM_WINDOWS 0)
set(PLATFORM_OSX 0)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(PLATFORM_LINUX 1)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(PLATFORM_WINDOWS 1)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(PLATFORM_OSX 1)
endif()

find_package(OpenGL REQUIRED)

set(VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)
set(DATA_DIR ${PROJECT_SOURCE_DIR}/data)

configure_file(
  "${PROJECT_SOURCE_DIR}/src/config_in.hpp"
  "${PROJECT_BINARY_DIR}/include/config.hpp"
)

include_directories(
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/src"
  "${VENDOR_DIR}/include"
)

link_directories("${VENDOR_DIR}/lib")

function(vendor_static_lib var_name lib_name)
  find_library(
    ${var_name}
    NAMES "${lib_name}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    PATHS "${VENDOR_DIR}/lib"
    NO_DEFAULT_PATH
  )
endfunction()

vendor_static_lib(libglew libGLEW)
vendor_static_lib(libglfw libglfw3)

if (PLATFORM_LINUX)
  set(
    PLATFORM_LIBS
    -lXrandr -lXext -lX11 -lXinerama -lXcursor -lrt -ldl -lXxf86vm -pthread
  )
elseif (PLATFORM_WINDOWS)
  # TODO
  set(PLATFORM_LIBS)
elseif (PLATFORM_OSX)
  # TODO
  set(PLATFORM_LIBS)
endif()

set(
  ALL_LIBS
  ${libglew}
  ${libglfw}
  ${OPENGL_LIBRARY}
  ${PLATFORM_LIBS}
)

file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(mandelbrot ${srcs})

target_link_libraries(mandelbrot ${ALL_LIBS})

file(COPY "${DATA_DIR}" DESTINATION "${PROJECT_BINARY_DIR}")

