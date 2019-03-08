#pragma once

#include <map>
#include <string>
#include "gl.hpp"

extern const std::map<std::string, std::string> PRESETS;
const int DEFAULT_MAX_ITERATIONS = 100;
const std::string DEFAULT_COLOUR_SCHEME = "coloured";

class OfflineRenderStatus {
  friend class Mandelbrot;

public:
  OfflineRenderStatus() {}
  OfflineRenderStatus(int w, int h, int stripH);

  int w = 0;
  int h = 0;
  int progress = 0;
  uint8_t* data = nullptr;

private:
  int stripsDrawn = 0;
  int stripH = 0;
  int totalStrips = 0;
  int finalStripH = 0;
};

class Mandelbrot {
public:
  Mandelbrot();

  void initialise(int w, int h);

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
  const OfflineRenderStatus& continueOfflineRender();

private:
  bool m_initialised = false;

  struct {
    GLuint id = 0;

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

  GLuint m_texProgram = 0;
  GLuint m_texture = 0;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;

  OfflineRenderStatus m_offlineRenderStatus;

  struct {
    int w;
    int h;
    int maxIterations;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
  } m_renderParams, m_renderParamsBackup;

  std::string m_mandelbrotVertShaderPath;
  std::string m_mandelbrotFragShaderPath;
  std::string m_texVertShaderPath;
  std::string m_texFragShaderPath;

  std::string m_activeComputeColourImpl;

  void initUniforms();
  void updateUniforms();
  void render();
  void drawFromTexture();
  void compileProgram_(const std::string& computeColourImpl);
  GLuint renderToTexture(int w, int h);
  void renderStripToMainMemoryBuffer(uint8_t* buffer);
};
