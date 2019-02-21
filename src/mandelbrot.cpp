#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "config.hpp"

using std::string;
using std::vector;

GLuint loadShader(const string& srcPath, GLuint type) {
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
    std::cerr << errMsg.data() << std::endl;
  }

  return shaderId;
}

GLuint loadShaders(const string& vertShaderPath, const string& fragShaderPath) {
  GLuint vertShader = loadShader(vertShaderPath, GL_VERTEX_SHADER);
  GLuint fragShader = loadShader(fragShaderPath, GL_FRAGMENT_SHADER);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  glGetProgramiv(program, GL_LINK_STATUS, &result);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    glGetProgramInfoLog(program, infoLogLen, NULL, errMsg.data());
    std::cerr << errMsg.data() << std::endl;
  }

  glDetachShader(program, vertShader);
  glDetachShader(program, fragShader);

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  return program;
}

int main() {
  std::cout << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
    << Mandelbrot_VERSION_MINOR << std::endl;

  glewExperimental = true;

  if (!glfwInit()) {
    std::cerr << "Failed to initialise GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = nullptr;
  window = glfwCreateWindow(1024, 768, "Mandelbrot", NULL, NULL);

  if (window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return 1;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f
  };

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData,
               GL_STATIC_DRAW);

  GLuint program = loadShaders("data/vert_shader.glsl",
                               "data/frag_shader.glsl");

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  return 0;
}

