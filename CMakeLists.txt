cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mandelbrot CXX)

set(Mandelbrot_VERSION_MAJOR 0)
set(Mandelbrot_VERSION_MINOR 1)

set(vendor_dir ${PROJECT_SOURCE_DIR}/vendor)

configure_file(
  "${PROJECT_SOURCE_DIR}/src/config_in.hpp"
  "${PROJECT_BINARY_DIR}/include/config.hpp"
)

include_directories(
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/vendor/include"
)

link_directories("${PROJECT_SOURCE_DIR}/vendor/lib")

function(vendor_static_lib var_name lib_name)
  find_library(
    ${var_name}
    NAMES "${lib_name}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    PATHS "${PROJECT_SOURCE_DIR}/vendor/lib"
    NO_DEFAULT_PATH
  )
endfunction()

vendor_static_lib(libglew libGLEW)
vendor_static_lib(libglfw libglfw3)

file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(mandelbrot ${srcs})

target_link_libraries(mandelbrot "${libglew}" "${libglfw}")

