#pragma once

#include "mandelbrot.hpp"

class Renderer {
public:
  Renderer(std::function<void()> fnMakeGlContextCurrent);

  void initialise(int w, int h);
  bool isInitialised() const;
  void resize(int w, int h);

  void clear(uint8_t r, uint8_t g, uint8_t b);
  void drawMandelbrot(bool fromTexture = false);
  void drawSelectionRect(double x, double y, double w, double h);
  void finish();

  void graphSpaceZoom(double x, double y, double mag);
  void screenSpaceZoom(double x, double y, double mag);
  void screenSpaceZoom(double x0, double y0, double x1, double y1);
  void resetZoom();

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
  double m_w;
  double m_h;
  bool m_selectionRectVisible = false;
  std::function<void()> m_fnMakeGlContextCurrent;

  Mandelbrot m_brot;

  struct {
    GLuint id;

    // Uniforms
    struct {
      GLuint colour;
    } u;

    GLuint vao;
    GLuint vbo;
  } m_program;

  std::string m_vertShaderPath;
  std::string m_fragShaderPath;

  void initUniforms();
  void updateUniforms(const float colour[3]);
  void makeSelectionRect(double x, double y, double w, double h);
  void makeRectangle(double x, double y, double w, double h,
                     GLfloat* verts) const;
  double screenToClipSpaceX(double x) const;
  double screenToClipSpaceY(double y) const;
};
