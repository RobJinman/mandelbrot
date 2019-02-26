#pragma once

#include <string>
#include <map>
#include "gl.hpp"

extern const std::map<std::string, std::string> PRESETS;
const int DEFAULT_MAX_ITERATIONS = 100;
const std::string DEFAULT_COLOUR_SCHEME = "coloured";

class Mandelbrot {
public:
  Mandelbrot(int W, int H);

  void init();
  void resize(int w, int y);
  void draw();
  void zoom(double x, double y, double mag);
  void reset();

  void setMaxIterations(int maxI);
  void setColourScheme(const std::string& presetName);
  void setColourSchemeImpl(const std::string& computeColourImpl);

  double getXMin() const;
  double getXMax() const;
  double getYMin() const;
  double getYMax() const;
  int getMaxIterations() const;

  double computeMagnification() const;

  uint8_t* renderToMainMemoryBuffer(int w, int h, size_t& bytes);

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
