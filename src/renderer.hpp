#pragma once

#include "mandelbrot.hpp"

class Renderer {
public:
  Renderer(int w, int h);

  void initialise();
  bool isInitialised() const;
  void resize(int w, int h);
  void draw(bool fromTexture = false);
  void drawSelectionRect(double x, double y, double w, double h);

  Mandelbrot brot;

private:
  bool m_initialised = false;
  double m_w;
  double m_h;

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
