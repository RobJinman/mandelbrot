#include "canvas.hpp"
#include "mandelbrot.hpp"

namespace chrono = std::chrono;

const double TARGET_FPS = 10.0;
const double ZOOM_PER_FRAME = 1.05;

wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
  EVT_PAINT(Canvas::onPaint)
  EVT_SIZE(Canvas::onResize)
  EVT_KEY_DOWN(Canvas::onKeyPress)
  EVT_TIMER(wxID_ANY, Canvas::onTick)
wxEND_EVENT_TABLE()

Canvas::Canvas(wxFrame* parent, const int* args, Mandelbrot& mandelbrot)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition,
               wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    m_mandelbrot(mandelbrot) {

  wxGLContextAttrs attrs;
  attrs.MajorVersion(3).MinorVersion(3).ForwardCompatible();
  m_context.reset(new wxGLContext(this, nullptr, &attrs));

  m_timer = new wxTimer(this);

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
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

void Canvas::onKeyPress(wxKeyEvent& e) {
  auto key = e.GetKeyCode();

  if (key == 'F') {
    std::cout << m_measuredFrameRate << "\n";
  }
  else if (key == 'Z') {
    if (!m_flyThroughMode) {
      std::cout << "Activating flythough mode\n";

      centreCursor();
      m_timer->Start(1000.0 / TARGET_FPS);
      m_flyThroughMode = true;
    }
    else {
      std::cout << "Deactivating flythough mode\n";

      m_timer->Stop();
      m_flyThroughMode = false;
    }
  }
}

void Canvas::initGl() {
  if (!IsShownOnScreen()) {
    return;
  }

  wxGLCanvas::SetCurrent(*m_context);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  m_mandelbrot.init();

  resize();

  m_initialised = true;
}

void Canvas::measureFrameRate() {
  if (m_frame % 10 == 0) {
    chrono::high_resolution_clock::time_point t_ = chrono::high_resolution_clock::now();
    chrono::duration<double> span = chrono::duration_cast<chrono::duration<double>>(t_ - m_t);
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
    m_mandelbrot.zoom(p.x, p.y, ZOOM_PER_FRAME);
  }

  wxGLCanvas::SetCurrent(*m_context);
  m_mandelbrot.draw();

  measureFrameRate();

  glFlush();
  SwapBuffers();
}
