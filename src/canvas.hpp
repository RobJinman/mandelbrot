#pragma once

#include <memory>
#include <chrono>
#include <wx/wx.h>
#include "gl.hpp"

class Mandelbrot;

class Canvas : public wxGLCanvas {
public:
  Canvas(wxFrame* parent, const int* args, Mandelbrot& mandelbrot);

private:
  void initGl();
  void render(wxDC& dc);
  void measureFrameRate();
  void centreCursor();
  wxPoint getCursorPos() const;
  wxPoint dampenCursorPos(const wxPoint& p) const;
  void resize();
  void activateFlyThroughMode();
  void deactivateFlyThroughMode();

  void onResize(wxSizeEvent& e);
  void onKeyPress(wxKeyEvent& e);
  void onPaint(wxPaintEvent& e);
  void onTick(wxTimerEvent& e);

  bool m_initialised = false;
  wxTimer* m_timer;
  Mandelbrot& m_mandelbrot;
  std::unique_ptr<wxGLContext> m_context;
  long long m_frame = 0;
  double m_measuredFrameRate = 0.0;
  std::chrono::high_resolution_clock::time_point m_t =
    std::chrono::high_resolution_clock::now();
  bool m_flyThroughMode = false;

  wxDECLARE_EVENT_TABLE();
};

enum ToggleStatus {
  TOGGLED_OFF = 0,
  TOGGLED_ON = 1
};

wxDECLARE_EVENT(FLY_THROUGH_MODE_TOGGLED, wxCommandEvent);
