#pragma once

#include <memory>
#include <wx/wx.h>
#include "gl.hpp"

class Mandelbrot;

class Canvas : public wxGLCanvas {
public:
  Canvas(wxFrame* parent, const int* args, Mandelbrot& mandelbrot);

  void resized(wxSizeEvent& e);
  void render(wxPaintEvent& e);

private:
  void initGl();

  bool m_initialised = false;
  Mandelbrot& m_mandelbrot;
  std::unique_ptr<wxGLContext> m_context;

  wxDECLARE_EVENT_TABLE();
};
