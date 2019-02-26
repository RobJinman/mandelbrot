#pragma once

#include <functional>
#include <memory>
#include <chrono>
#include <wx/wx.h>
#include "gl.hpp"

const double DEFAULT_TARGET_FPS = 10.0;
const double DEFAULT_ZOOM_PER_FRAME = 1.025;

class Mandelbrot;

class Canvas : public wxGLCanvas {
public:
  Canvas(wxWindow* parent, const int* args, Mandelbrot& mandelbrot,
         std::function<void()> onRender);

  void refresh();
  void setTargetFps(double fps);
  void setZoomPerFrame(double zoom);

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
  void onLeftMouseBtnDown(wxMouseEvent& e);
  void onLeftMouseBtnUp(wxMouseEvent& e);
  void onMouseMove(wxMouseEvent& e);
  void onPaint(wxPaintEvent& e);
  void onTick(wxTimerEvent& e);

  bool m_initialised = false;
  wxTimer* m_timer;
  std::function<void()> m_onRender;
  Mandelbrot& m_mandelbrot;
  std::unique_ptr<wxGLContext> m_context;
  long long m_frame = 0;
  double m_measuredFrameRate = 0.0;
  std::chrono::high_resolution_clock::time_point m_t =
    std::chrono::high_resolution_clock::now();
  bool m_flyThroughMode = false;
  double m_targetFps;
  double m_zoomPerFrame;

  bool m_mouseDown = false;
  wxPoint m_mouseOrigin;

  wxDECLARE_EVENT_TABLE();
};

enum ToggleStatus {
  TOGGLED_OFF = 0,
  TOGGLED_ON = 1
};

wxDECLARE_EVENT(FLY_THROUGH_MODE_TOGGLED, wxCommandEvent);
