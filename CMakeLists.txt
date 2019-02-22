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
  "${VENDOR_DIR}/include/wx-3.1"
  "${VENDOR_DIR}/lib/wx/include/gtk2-unicode-static-3.1"
)

link_directories("${VENDOR_DIR}/lib")

function(find_vendor_libs RESULT LIB_NAMES)
  set(LIB_PATHS "")
  foreach(LIB_NAME IN LISTS LIB_NAMES)
    set(LIB_VAR "LIB_${LIB_NAME}")
    find_library(
      ${LIB_VAR}
      NAMES "${LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
      PATHS "${VENDOR_DIR}/lib"
      NO_DEFAULT_PATH
    )
    list(APPEND LIB_PATHS "${${LIB_VAR}}")
  endforeach()
  set(${RESULT} "${LIB_PATHS}" PARENT_SCOPE)
endfunction()

set(VENDOR_LIB_NAMES libGLEW
                     libglfw3
                     libwx_gtk2u_gl-3.1
                     libwx_gtk2u_core-3.1
                     libwx_baseu-3.1)

find_vendor_libs(VENDOR_STATIC_LIBS "${VENDOR_LIB_NAMES}")

if (PLATFORM_LINUX)
  set(
    PLATFORM_LIBS
    -lXrandr -lXext -lX11 -lXinerama -lXcursor -lrt -ldl -lXxf86vm -pthread
  )
  set(WX_OPTIONS -DWXUSINGDLL -D__WXGTK2__ -D__WXGTK__)
  set(WX_DEPS -lSM -lICE -lX11 -lXext -lglib-2.0
      -lgobject-2.0 -latk-1.0 -lgio-2.0 -lgthread-2.0
      -lgmodule-2.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0
      -lpangocairo-1.0 -lpangoft2-1.0 -lpangoxft-1.0
      -lpng -lgdk-x11-2.0 -lgtk-x11-2.0 -lz)
elseif (PLATFORM_WINDOWS)
  # TODO
  set(PLATFORM_LIBS)
elseif (PLATFORM_OSX)
  # TODO
  set(PLATFORM_LIBS)
endif()

set(
  ALL_LIBS
  ${VENDOR_STATIC_LIBS}
  ${WX_DEPS}
  ${OPENGL_LIBRARY}
  ${PLATFORM_LIBS}
)

file(GLOB CPP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(mandelbrot ${CPP_SOURCES})

target_compile_options(mandelbrot PRIVATE ${WX_OPTIONS})
target_link_libraries(mandelbrot ${ALL_LIBS})

file(COPY "${DATA_DIR}" DESTINATION "${PROJECT_BINARY_DIR}")
