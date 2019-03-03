#include "canvas.hpp"
#include "exception.hpp"
#include "renderer.hpp"
#include "wx_helpers.hpp"

namespace chrono = std::chrono;

wxDEFINE_EVENT(FLY_THROUGH_MODE_TOGGLED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
  EVT_PAINT(Canvas::onPaint)
  EVT_SIZE(Canvas::onResize)
  EVT_KEY_DOWN(Canvas::onKeyPress)
  EVT_LEFT_DOWN(Canvas::onLeftMouseBtnDown)
  EVT_LEFT_UP(Canvas::onLeftMouseBtnUp)
  EVT_MOTION(Canvas::onMouseMove)
  EVT_TIMER(wxID_ANY, Canvas::onTick)
wxEND_EVENT_TABLE()

static bool selectionSizeAboveThreshold(const wxSize& sz) {
  return sz.x * sz.x + sz.y * sz.y >= 64;
}

Canvas::Canvas(wxWindow* parent, const wxGLAttributes& glAttrs,
               Renderer& renderer, std::function<void()> onRender)
  : wxGLCanvas(parent, glAttrs, wxID_ANY, wxDefaultPosition, wxDefaultSize,
               wxFULL_REPAINT_ON_RESIZE),
    m_renderer(renderer),
    m_onRender(onRender) {

  m_targetFps = DEFAULT_TARGET_FPS;
  m_zoomPerFrame = DEFAULT_ZOOM_PER_FRAME;
  m_zoomAmount = DEFAULT_ZOOM;

#ifdef __APPLE__
  wxGLContextAttrs attrs;
  attrs.PlatformDefaults()
       .CoreProfile()
       .MajorVersion(GL_VERSION_MAJOR)
       .MinorVersion(GL_VERSION_MINOR)
       .ForwardCompatible()
       .EndList();
#else
  wxGLContextAttrs attrs;
  attrs.MajorVersion(GL_VERSION_MAJOR)
       .MinorVersion(GL_VERSION_MINOR)
       .ForwardCompatible()
       .EndList();
#endif
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

void Canvas::setZoomAmount(double zoom) {
  m_zoomAmount = zoom;
}

void Canvas::onTick(wxTimerEvent&) {
  if (m_flyThroughMode) {
    auto p = dampenCursorPos(getCursorPos());
    m_renderer.zoom(p.x, p.y, m_zoomPerFrame);
  }

  render();
}

void Canvas::onResize(wxSizeEvent& e) {
  resize();
  e.Skip();
}

void Canvas::resize() {
  auto sz = GetSize();
  m_renderer.resize(sz.x, sz.y);

  refresh();
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

void Canvas::onLeftMouseBtnDown(wxMouseEvent& e) {
  SetFocus();
  SetCurrent(*m_context);

  m_mouseDown = true;
  m_selectionRect = wxRect(e.GetPosition(), wxSize(0, 0));
}

void Canvas::onLeftMouseBtnUp(wxMouseEvent&) {
  m_mouseDown = false;

  if (selectionSizeAboveThreshold(m_selectionRect.GetSize())) {
    int x0 = m_selectionRect.x;
    int y0 = m_selectionRect.y;
    int x1 = x0 + m_selectionRect.width;
    int y1 = y0 + m_selectionRect.height;

    m_renderer.zoom(x0, y0, x1, y1);
  }

  m_renderer.drawSelectionRect(0, 0, 0, 0);
  refresh();
}

void Canvas::onMouseMove(wxMouseEvent&) {
  if (m_mouseDown) {
    wxPoint p = wxGetMousePosition() - GetScreenPosition();
    wxPoint sz_p = p - m_selectionRect.GetTopLeft();
    wxSize sz(sz_p.x, sz_p.y);

    wxSize winSz = GetClientSize();
    float aspect = static_cast<float>(winSz.x) / static_cast<float>(winSz.y);
    sz.y = sz.x / aspect;

    m_selectionRect.SetSize(sz);

    refresh();
  }
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
  else if (key == 'R') {
    m_renderer.resetZoom();
    refresh();
  }
  else if (key == WXK_SPACE) {
    auto sz = GetClientSize();
    m_renderer.zoom(sz.x / 2, sz.y / 2, m_zoomAmount);
    refresh();
  }
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

void Canvas::onPaint(wxPaintEvent&) {
  if (!m_renderer.isInitialised()) {
    SetCurrent(*m_context);
    m_renderer.initialise();
    resize();
  }

  render();
}

void Canvas::refresh() {
  Refresh();
  Update();
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

void Canvas::makeGlContextCurrent() {
  SetCurrent(*m_context);
}

void Canvas::render() {
  if (!IsShownOnScreen()) {
    return;
  }

  SetCurrent(*m_context);

  if (m_mouseDown) {
    int x = m_selectionRect.x;
    int y = m_selectionRect.y;
    int w = m_selectionRect.width;
    int h = m_selectionRect.height;

    m_renderer.drawSelectionRect(x, y, w, h);
  }

  m_renderer.draw(m_mouseDown);

  measureFrameRate();

  SwapBuffers();

  m_onRender();
}
