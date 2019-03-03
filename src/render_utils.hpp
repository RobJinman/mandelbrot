#pragma once

#include <string>
#include "gl.hpp"

GLuint loadShader(const std::string& path, GLuint type);
GLuint loadShaderWithSubstitution(const std::string& path, GLuint type,
                                  const std::string& searchString,
                                  const std::string& substitution);
GLuint compileProgram(const std::string& vertShaderPath,
                      const std::string& fragShaderPath,
                      const std::string& searchString = "",
                      const std::string& substitution = "");
