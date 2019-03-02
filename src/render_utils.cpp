#include <vector>
#include <fstream>
#include "render_utils.hpp"
#include "exception.hpp"

using std::string;
using std::vector;

static string doSubstitution(const string& path, const string& searchString,
                             const string& substitution) {

  std::ifstream fin(path);
  string text{std::istreambuf_iterator<char>(fin),
              std::istreambuf_iterator<char>()};

  if (searchString.length() > 0) {
    auto idx = text.find(searchString);
    auto len = searchString.length();
    text.replace(idx, len, substitution);
  }

  return text;
}

GLuint loadShaderWithSubstitution(const string& srcPath, GLuint type,
                                  const string& searchString,
                                  const string& substitution) {
  GLuint shaderId = GL_CHECK(glCreateShader(type));

  string shaderSrc = doSubstitution(srcPath, searchString, substitution);

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  const char* srcPtr = shaderSrc.c_str();
  GL_CHECK(glShaderSource(shaderId, 1, &srcPtr, NULL));
  GL_CHECK(glCompileShader(shaderId));

  GL_CHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
  GL_CHECK(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLen));
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    GL_CHECK(glGetShaderInfoLog(shaderId, infoLogLen, NULL, errMsg.data()));
    throw ShaderException(errMsg.data());
  }

  return shaderId;
}

GLuint loadShader(const string& srcPath, GLuint type) {
  return loadShaderWithSubstitution(srcPath, type, "", "");
}
