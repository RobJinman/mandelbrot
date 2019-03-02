#pragma once

#include <stdexcept>
#include <sstream>
#include "gl.hpp"

#define EXCEPTION(msg) \
  { \
    std::stringstream ss; \
    ss << msg << " (FILE: " << __FILE__ << ", LINE: " << __LINE__ << ")" \
       << std::endl; \
    throw std::runtime_error(ss.str()); \
  }

#define GL_SUPPORT_EXCEPTION(msg) \
  { \
    std::stringstream ss; \
    ss << msg << " (FILE: " << __FILE__ << ", LINE: " << __LINE__ << ")" \
       << std::endl; \
    throw MissingGlSupportException(ss.str()); \
  }

#define GL_EXCEPTION(msg, code) \
  { \
    std::stringstream ss; \
    ss << msg << " (code=0x" << std::hex << code << std::dec << ") (FILE: " \
       << __FILE__ << ", LINE: " << __LINE__ << ")" << std::endl; \
    throw OpenGlException(ss.str(), code); \
  }

#define GL_CHECK(x) \
  x; \
  { \
    GLenum glError = glGetError(); \
    if (glError != GL_NO_ERROR) { \
      std::stringstream ss; \
      ss << "OpenGL error, code=0x" << std::hex << glError << std::dec \
        << " (FILE: " << __FILE__ << ", LINE: " << __LINE__ << ")" \
        << std::endl; \
      throw OpenGlException(ss.str(), glError); \
    } \
  }

class ShaderException : public std::runtime_error {
public:
  ShaderException(const std::string& errorOutput)
    : runtime_error("Shader failed to compile"),
      m_errorOutput(errorOutput) {}

  const std::string& errorOutput() const {
    return m_errorOutput;
  }

private:
  const std::string m_errorOutput;
};

class OpenGlException : public std::runtime_error {
public:
  OpenGlException(const std::string& msg, GLenum code)
    : runtime_error(msg),
      code(code) {}

  const GLenum code;
};

class MissingGlSupportException : public std::runtime_error {
public:
  MissingGlSupportException(const std::string& msg)
    : runtime_error(msg) {}
};
