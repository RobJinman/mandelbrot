#pragma once

#include "mandelbrot.hpp"

class Renderer {
public:
  Renderer(int w, int h);

  void initialise();
  bool isInitialised() const;
  void resize(int w, int h);
  void drawSelectionRect(double x0, double y0, double x1, double y1);

  Mandelbrot brot;

private:
  bool m_initialised = false;
};
