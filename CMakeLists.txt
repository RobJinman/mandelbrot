cmake_minimum_required(VERSION 3.10)

project(Mandelbrot CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(common)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Mandelbrot_VERSION_MAJOR 0)
set(Mandelbrot_VERSION_MINOR 1)

find_package(OpenGL REQUIRED)

set(VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor/${PLATFORM_NAME})
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

message("${VENDOR_DIR}/include")

link_directories("${VENDOR_DIR}/lib")

function(find_vendor_libs RESULT LIB_NAMES)
  set(LIB_PATHS "")
  foreach(LIB_NAME IN LISTS LIB_NAMES)
    set(LIB_VAR "LIB_${LIB_NAME}")
    find_library(
      ${LIB_VAR}
      NAMES "${LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
      PATHS "${VENDOR_DIR}/lib"
      PATH_SUFFIXES "vc_x64_lib"
      NO_DEFAULT_PATH
    )
    list(APPEND LIB_PATHS "${${LIB_VAR}}")
  endforeach()
  set(${RESULT} "${LIB_PATHS}" PARENT_SCOPE)
endfunction()

if (PLATFORM_LINUX)
  set(VENDOR_LIB_NAMES libGLEW
                       libwx_gtk2u_richtext-3.1
                       libwx_gtk2u_html-3.1
                       libwx_baseu_xml-3.1
                       libwx_gtk2u_gl-3.1
                       libwx_gtk2u_core-3.1
                       libwx_baseu-3.1)
elseif (PLATFORM_WINDOWS)
  set(VENDOR_LIB_NAMES libGLEW32
                       wxmsw31u_gl
                       wxmsw31u_core
                       wxpng
                       wxtiff
                       wxscintilla
                       wxzlib
                       wxregexu
                       wxexpat
                       wxbase31u)
endif()

find_vendor_libs(VENDOR_STATIC_LIBS "${VENDOR_LIB_NAMES}")

if (PLATFORM_LINUX)
  set(
    PLATFORM_LIBS
    -lXrandr -lXext -lX11 -lXinerama -lXcursor -lrt -ldl -lXxf86vm -pthread
    # From wx-config
    -lSM -lICE -lX11 -lXext -lglib-2.0
    -lgobject-2.0 -latk-1.0 -lgio-2.0 -lgthread-2.0
    -lgmodule-2.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0
    -lpangocairo-1.0 -lpangoft2-1.0 -lpangoxft-1.0
    -lpng -lgdk-x11-2.0 -lgtk-x11-2.0 -lexpat -lz
  )
  set(PLATFORM_COMPILE_FLAGS -DWXUSINGDLL -D__WXGTK2__ -D__WXGTK__)
  set(PLATFORM_LINK_FLAGS)

  include_directories(
    "${VENDOR_DIR}/include/wx-3.1"
    "${VENDOR_DIR}/lib/wx/include/gtk2-unicode-static-3.1"
  )
elseif (PLATFORM_WINDOWS)
  set(PLATFORM_LIBS -defaultlib:comctl32.lib -defaultlib:rpcrt4.lib)
  set(PLATFORM_COMPILE_FLAGS -DGLEW_STATIC -DwxUSE_OLE)
  set(PLATFORM_LINK_FLAGS /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup)

  include_directories(
    "${VENDOR_DIR}/lib/vc_x64_lib/mswu"
  )
elseif (PLATFORM_OSX)
  # TODO
  set(PLATFORM_LIBS)
  set(PLATFORM_COMPILE_FLAGS)
  set(PLATFORM_LINK_FLAGS)

  include_directories(
    "${VENDOR_DIR}/include/wx-3.1"
    "${VENDOR_DIR}/lib/wx/include/osx_cocoa-unicode-static-3.1"
  )
endif()

set(
  ALL_LIBS
  ${VENDOR_STATIC_LIBS}
  ${OPENGL_LIBRARY}
  ${PLATFORM_LIBS}
)

file(GLOB CPP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(mandelbrot ${CPP_SOURCES})

target_compile_options(
  mandelbrot
  PRIVATE ${WX_OPTIONS} ${GLEW_OPTIONS} ${PLATFORM_COMPILE_FLAGS}
)
target_link_libraries(mandelbrot ${ALL_LIBS})
target_link_options(mandelbrot PRIVATE ${PLATFORM_LINK_FLAGS})

file(COPY "${DATA_DIR}" DESTINATION "${PROJECT_BINARY_DIR}")
