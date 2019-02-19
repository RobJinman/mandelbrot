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
  "${vendor_dir}/include"
)

file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(mandelbrot ${srcs})

