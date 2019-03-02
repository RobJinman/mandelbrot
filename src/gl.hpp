#pragma once

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#include <GL/glew.h>

#ifdef __APPLE__
  #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
  #include <OpenGL/gl3.h>
#endif

#include <wx/glcanvas.h>

#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 3

#define GL_VERSION_STRING \
  "GL_VERSION_" TO_STRING(GL_VERSION_MAJOR) "_" TO_STRING(GL_VERSION_MINOR)
