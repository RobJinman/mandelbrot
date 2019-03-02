#pragma once

#include <map>
#include <string>
#include "gl.hpp"

extern const std::map<std::string, std::string> PRESETS;
const int DEFAULT_MAX_ITERATIONS = 100;
const std::string DEFAULT_COLOUR_SCHEME = "coloured";

class Mandelbrot {
public:
  Mandelbrot(int W, int H);

  void initialise();

  void resize(int w, int y);
  void draw();

  void zoom(double x, double y, double mag);
  void zoom(double x0, double y0, double x1, double y1);
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
  bool m_initialised = false;

  struct {
    GLuint id;

    // Uniforms
    struct {
      GLuint w;
      GLuint h;
      GLuint maxIterations;
      GLuint xmin;
      GLuint xmax;
      GLuint ymin;
      GLuint ymax;
    } u;

    GLuint vertexBufferId;
  } m_program;

  int m_w;
  int m_h;
  int m_maxIterations;
  double m_xmin;
  double m_xmax;
  double m_ymin;
  double m_ymax;

  std::string m_vertShaderPath;
  std::string m_fragShaderPath;
  std::string m_activeComputeColourImpl;

  void compileProgram(const std::string& computeColourImpl);
  void initUniforms();
  void updateUniforms();
};
