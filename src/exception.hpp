#pragma once

#include <stdexcept>
#include <sstream>

#define EXCEPTION(msg) \
  { \
    std::stringstream ss; \
    ss << msg << " (FILE: " << __FILE__ << ", LINE: " << __LINE__ << ")" \
       << std::endl; \
    throw std::runtime_error(ss.str()); \
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
