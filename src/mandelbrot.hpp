#pragma once

#include <string>
#include "gl.hpp"

const int DEFAULT_MAX_ITERATIONS = 100;

class Mandelbrot {
public:
  Mandelbrot(int W, int H);

  void init();
  void resize(int w, int y);
  void draw();
  void zoom(double x, double y, double mag);

  void setMaxIterations(int maxI);
  void setColourScheme(const std::string& computeColourImpl);

private:
  void loadShaders(const std::string& fragShaderPath,
                   const std::string& vertShaderPath,
                   const std::string& computeColourImpl);
  GLuint loadShader(const std::string& path, GLuint type,
                    const std::string& computeColourImpl = "");
  void initUniforms();
  void updateUniforms();

  bool m_initialised = false;
  int m_W;
  int m_H;
  int m_maxIterations;
  double m_xmin;
  double m_xmax;
  double m_ymin;
  double m_ymax;
  std::string m_activeComputeColourImpl;

  struct {
    GLuint uW;
    GLuint uH;
    GLuint uMaxIterations;
    GLuint uXmin;
    GLuint uXmax;
    GLuint uYmin;
    GLuint uYmax;
  } m_uniforms;

  GLuint m_vertexBuffer;
  GLuint m_program;
};
