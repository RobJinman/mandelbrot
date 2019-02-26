#include "canvas.hpp"
#include "exception.hpp"
#include "mandelbrot.hpp"

namespace chrono = std::chrono;

wxDEFINE_EVENT(FLY_THROUGH_MODE_TOGGLED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
  EVT_PAINT(Canvas::onPaint)
  EVT_SIZE(Canvas::onResize)
  EVT_KEY_DOWN(Canvas::onKeyPress)
  EVT_TIMER(wxID_ANY, Canvas::onTick)
wxEND_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent, const int* args, Mandelbrot& mandelbrot,
               std::function<void()> onRender)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition,
               wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    m_mandelbrot(mandelbrot),
    m_onRender(onRender) {

  m_targetFps = DEFAULT_TARGET_FPS;
  m_zoomPerFrame = DEFAULT_ZOOM_PER_FRAME;

  wxGLContextAttrs attrs;
  attrs.MajorVersion(3).MinorVersion(3).ForwardCompatible();
  m_context.reset(new wxGLContext(this, nullptr, &attrs));

  m_timer = new wxTimer(this);

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

void Canvas::setTargetFps(double fps) {
  m_targetFps = fps;
}

void Canvas::setZoomPerFrame(double zoom) {
  m_zoomPerFrame = zoom;
}

void Canvas::onTick(wxTimerEvent& e) {
  wxClientDC dc(this);
  render(dc);
}

void Canvas::onResize(wxSizeEvent&) {
  resize();
}

void Canvas::resize() {
  auto sz = GetSize();
  m_mandelbrot.resize(sz.x, sz.y);

  Refresh();
}

void Canvas::centreCursor() {
  auto p = getCursorPos();
  WarpPointer(p.x, p.y);
}

wxPoint Canvas::getCursorPos() const {
  auto pt = wxGetMousePosition();
  return pt - GetScreenPosition();
}

void Canvas::activateFlyThroughMode() {
  centreCursor();
  m_timer->Start(1000.0 / m_targetFps);
  m_flyThroughMode = true;

  wxCommandEvent event(FLY_THROUGH_MODE_TOGGLED);
  event.SetInt(TOGGLED_ON);
  wxPostEvent(this, event);
}

void Canvas::deactivateFlyThroughMode() {
  m_timer->Stop();
  m_flyThroughMode = false;

  wxCommandEvent event(FLY_THROUGH_MODE_TOGGLED);
  event.SetInt(TOGGLED_OFF);
  wxPostEvent(this, event);
}

void Canvas::onKeyPress(wxKeyEvent& e) {
  auto key = e.GetKeyCode();

  if (key == 'F') {
    std::cout << m_measuredFrameRate << std::endl;
  }
  else if (key == 'Z') {
    if (m_flyThroughMode) {
      deactivateFlyThroughMode();
    }
    else {
      activateFlyThroughMode();
    }
  }
}

void Canvas::initGl() {
  if (!IsShownOnScreen()) {
    return;
  }

  wxGLCanvas::SetCurrent(*m_context);

  glewExperimental = GL_TRUE;
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    EXCEPTION("Failed to initialize GLEW: " << glewGetErrorString(result));
  }

  m_mandelbrot.init();

  resize();

  m_initialised = true;
}

void Canvas::measureFrameRate() {
  if (m_frame % 10 == 0) {
    chrono::high_resolution_clock::time_point t_ =
      chrono::high_resolution_clock::now();
    chrono::duration<double> span = 
      chrono::duration_cast<chrono::duration<double>>(t_ - m_t);
    m_measuredFrameRate = 10.0 / span.count();
    m_t = t_;
  }
  ++m_frame;
}

void Canvas::onPaint(wxPaintEvent& e) {
  wxPaintDC dc(this);
  render(dc);
}

wxPoint Canvas::dampenCursorPos(const wxPoint& p) const {
  auto sz = GetSize();

  double centreX = 0.5 * sz.x;
  double centreY = 0.5 * sz.y;
  double dx = p.x - centreX;
  double dy = p.y - centreY;

  double d = sqrt(pow(dx, 2) + pow(dy, 2));
  double sf = 0.5 * d / sz.y;

  wxPoint p_;
  p_.x = centreX + dx * sf;
  p_.y = centreY + dy * sf;

  return p_;
}

void Canvas::render(wxDC& dc) {
  if (!IsShownOnScreen()) {
    return;
  }

  if (!m_initialised) {
    initGl();
  }

  if (m_flyThroughMode) {
    auto p = dampenCursorPos(getCursorPos());
    m_mandelbrot.zoom(p.x, p.y, m_zoomPerFrame);
  }

  wxGLCanvas::SetCurrent(*m_context);
  m_mandelbrot.draw();

  measureFrameRate();

  glFlush();
  SwapBuffers();

  m_onRender();
}
