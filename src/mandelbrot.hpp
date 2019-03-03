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
  void draw(bool fromTexture);

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

  void renderToMainMemoryBuffer(int w, int h);
  bool getRenderResult(int& w, int& h, uint8_t*& data, size_t& nBytes);

private:
  bool m_initialised = false;
  bool m_busy = false;

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
  } m_program;

  GLuint m_texProgram;
  GLuint m_texture = 0;
  GLuint m_vao;
  GLuint m_vbo;

  int m_w;
  int m_h;
  int m_maxIterations;
  double m_xmin;
  double m_xmax;
  double m_ymin;
  double m_ymax;

  std::string m_mandelbrotVertShaderPath;
  std::string m_mandelbrotFragShaderPath;
  std::string m_texVertShaderPath;
  std::string m_texFragShaderPath;

  std::string m_activeComputeColourImpl;

  struct {
    int prevW = 0;
    int prevH = 0;
    int w = 0;
    int h = 0;
    GLuint texture = 0;
    GLsync sync;
  } m_offlineRender;

  void initUniforms();
  void updateUniforms();
  void render();
  void drawFromTexture();
  void compileProgram_(const std::string& computeColourImpl);
  GLuint renderToTexture(int w, int h);
};
