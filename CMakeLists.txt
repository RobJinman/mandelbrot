cmake_minimum_required(VERSION 3.10)

project(Mandelbrot CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(common)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Mandelbrot_VERSION_MAJOR 0)
set(Mandelbrot_VERSION_MINOR 2)
set(VERSION_STRING "${Mandelbrot_VERSION_MAJOR}.${Mandelbrot_VERSION_MINOR}")

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
  set(VENDOR_LIB_NAMES libglew32
                       wxmsw31u_richtext
                       wxmsw31u_html
                       wxbase31u_xml
                       wxmsw31u_gl
                       wxmsw31u_core
                       wxpng
                       wxtiff
                       wxscintilla
                       wxzlib
                       wxregexu
                       wxexpat
                       wxbase31u)
elseif (PLATFORM_OSX)
  set(VENDOR_LIB_NAMES libGLEW
                       libwx_osx_cocoau_richtext-3.1
                       libwx_osx_cocoau_html-3.1
                       libwx_baseu_xml-3.1
                       libwx_osx_cocoau_gl-3.1
                       libwx_osx_cocoau_core-3.1
                       libwxpng-3.1
                       libwxtiff-3.1
                       libwxscintilla-3.1
                       libwxregexu-3.1
                       libwx_baseu-3.1)
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
  set(PLATFORM_COMPILE_FLAGS_COMMON -DWXUSINGDLL
                                    -D__WXGTK2__
                                    -D__WXGTK__
                                    -Wall -Wextra)
  set(PLATFORM_COMPILE_FLAGS_RELEASE -O3)
  set(PLATFORM_COMPILE_FLAGS_DEBUG -g -O0)

  set(PLATFORM_LINK_FLAGS)

  include_directories(
    "${VENDOR_DIR}/include/wx-3.1"
    "${VENDOR_DIR}/lib/wx/include/gtk2-unicode-static-3.1"
  )
elseif (PLATFORM_WINDOWS)
  set(PLATFORM_LIBS -defaultlib:comctl32.lib -defaultlib:rpcrt4.lib)
  set(PLATFORM_COMPILE_FLAGS_COMMON -DGLEW_STATIC -DwxUSE_OLE)
  set(PLATFORM_COMPILE_FLAGS_RELEASE)
  set(PLATFORM_COMPILE_FLAGS_DEBUG)
  set(PLATFORM_LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup")

  include_directories(
    "${VENDOR_DIR}/lib/vc_x64_lib/mswu"
  )
elseif (PLATFORM_OSX)
  set(PLATFORM_LIBS "-framework AudioToolbox"
                    "-framework WebKit"
                    -lexpat -lz
                    "-framework Security"
                    "-framework Carbon"
                    "-framework Cocoa"
                    "-framework CoreFoundation"
                    "-framework IOKit" -pthread)
  set(PLATFORM_COMPILE_FLAGS_COMMON -DWXUSINGDLL
                                    -D__WXOSX_COCOA__
                                    -D__WXMAC__
                                    -D__WXOSX__)
  set(PLATFORM_COMPILE_FLAGS_RELEASE)
  set(PLATFORM_COMPILE_FLAGS_DEBUG)
  set(PLATFORM_LINK_FLAGS "-Wl,-F/System/Library/Frameworks")

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

if (PLATFORM_OSX)
  set(ICON_FILE "${CMAKE_SOURCE_DIR}/icons/mandelbrot.icns")

  set(MACOSX_BUNDLE true)
  set(MACOSX_BUNDLE_BUNDLE_NAME "Mandelbrot")
  set(MACOSX_BUNDLE_INFO_STRING "Mandelbrot ${VERSION_STRING}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_STRING}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_STRING}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_STRING}")
  set(MACOSX_BUNDLE_ICON_FILE "mandelbrot.icns")

  set_source_files_properties(
    "${ICON_FILE}"
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )

  add_executable(
    mandelbrot MACOSX_BUNDLE
    ${CPP_SOURCES}
    "${ICON_FILE}"
  )
else()
  add_executable(mandelbrot ${CPP_SOURCES})
endif()

target_compile_options(
  mandelbrot
  PRIVATE ${WX_OPTIONS}
          ${GLEW_OPTIONS}
          ${PLATFORM_COMPILE_FLAGS_COMMON}
          "$<$<CONFIG:RELEASE>:${PLATFORM_COMPILE_FLAGS_RELEASE}>"
          "$<$<CONFIG:DEBUG>:${PLATFORM_COMPILE_FLAGS_DEBUG}>"
)
target_link_libraries(mandelbrot ${ALL_LIBS})
set_target_properties(mandelbrot PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")

file(COPY "${DATA_DIR}" DESTINATION "${PROJECT_BINARY_DIR}")

if (PLATFORM_LINUX)
  # TODO
elseif (PLATFORM_WINDOWS)
  # TODO
elseif (PLATFORM_OSX)
  install(CODE "set(DATA_DIR \"${DATA_DIR}\")")
  install(SCRIPT "${CMAKE_MODULE_PATH}/osx_bundle.cmake")
endif()

include(InstallRequiredSystemLibraries)
