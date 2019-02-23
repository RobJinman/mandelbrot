#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include "mandelbrot.hpp"
#include "exception.hpp"

using std::string;
using std::vector;

Mandelbrot::Mandelbrot(int W, int H) {
  m_W = W;
  m_H = H;
  m_maxIterations = 250;
  m_xmin = -2.5;
  m_xmax = 1.5;
  m_ymin = -2.0;
  m_ymax = 2.0;
}

void Mandelbrot::resize(int w, int h) {
  if (!m_initialised) {
    return;
  }

  m_W = w;
  m_H = h;
  glViewport(0, 0, w, h);
  updateUniforms();
}

void Mandelbrot::init() {
  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f, // A
    1.0f, -1.0f, 0.0f,  // B
    1.0f, 1.0f, 0.0f,   // C
    -1.0f, -1.0f, 0.0f, // A
    1.0f, 1.0f, 0.0f,   // C
    -1.0f, 1.0f, 0.0f,  // D
  };

  glGenBuffers(1, &m_vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData,
               GL_STATIC_DRAW);

  loadShaders("data/vert_shader.glsl", "data/frag_shader.glsl");

  initUniforms();

  m_initialised = true;
}

void Mandelbrot::initUniforms() {
  m_uniforms.uW = glGetUniformLocation(m_program, "W");
  m_uniforms.uH = glGetUniformLocation(m_program, "H");
  m_uniforms.uMaxIterations = glGetUniformLocation(m_program, "maxIterations");
  m_uniforms.uXmin = glGetUniformLocation(m_program, "xmin");
  m_uniforms.uXmax = glGetUniformLocation(m_program, "xmax");
  m_uniforms.uYmin = glGetUniformLocation(m_program, "ymin");
  m_uniforms.uYmax = glGetUniformLocation(m_program, "ymax");

  updateUniforms();
}

GLuint Mandelbrot::loadShader(const string& srcPath, GLuint type) {
  GLuint shaderId = glCreateShader(type);

  std::ifstream fin(srcPath);
  string shaderSrc{std::istreambuf_iterator<char>(fin),
                   std::istreambuf_iterator<char>()};

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  const char* srcPtr = shaderSrc.c_str();
  glShaderSource(shaderId, 1, &srcPtr, NULL);
  glCompileShader(shaderId);

  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLen);
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    glGetShaderInfoLog(shaderId, infoLogLen, NULL, errMsg.data());
    EXCEPTION(errMsg.data());
  }

  return shaderId;
}

void Mandelbrot::loadShaders(const string& vertShaderPath,
                             const string& fragShaderPath) {
  GLuint vertShader = loadShader(vertShaderPath, GL_VERTEX_SHADER);
  GLuint fragShader = loadShader(fragShaderPath, GL_FRAGMENT_SHADER);

  m_program = glCreateProgram();
  glAttachShader(m_program, vertShader);
  glAttachShader(m_program, fragShader);
  glLinkProgram(m_program);

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  glGetProgramiv(m_program, GL_LINK_STATUS, &result);
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLen);
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    glGetProgramInfoLog(m_program, infoLogLen, NULL, errMsg.data());
    EXCEPTION(errMsg.data());
  }

  glDetachShader(m_program, vertShader);
  glDetachShader(m_program, fragShader);

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  glUseProgram(m_program);
}

void Mandelbrot::updateUniforms() {
  glUniform1f(m_uniforms.uW, m_W);
  glUniform1f(m_uniforms.uH, m_H);
  glUniform1i(m_uniforms.uMaxIterations, m_maxIterations);
  glUniform1f(m_uniforms.uXmin, m_xmin);
  glUniform1f(m_uniforms.uXmax, m_xmax);
  glUniform1f(m_uniforms.uYmin, m_ymin);
  glUniform1f(m_uniforms.uYmax, m_ymax);
}

void Mandelbrot::zoom(double x, double y, double mag) {
  if (!m_initialised) {
    return;
  }

  y = m_H - 1 - y;

  double xRange = m_xmax - m_xmin;
  double yRange = m_ymax - m_ymin;

  double centreX = m_xmin + xRange * x / m_W;
  double centreY = m_ymin + yRange * y / m_H;

  double sf = sqrt(mag);

  double xRangeNew = xRange / sf;
  double yRangeNew = yRange / sf;

  m_xmin = centreX - 0.5 * xRangeNew;
  m_xmax = centreX + 0.5 * xRangeNew;

  m_ymin = centreY - 0.5 * yRangeNew;
  m_ymax = centreY + 0.5 * yRangeNew;

  updateUniforms();
}

void Mandelbrot::draw() {
  if (!m_initialised) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(m_program);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);
}
