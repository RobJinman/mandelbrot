#include "canvas.hpp"
#include "exception.hpp"
#include "renderer.hpp"
#include "wx_helpers.hpp"
#include "defaults.hpp"

namespace chrono = std::chrono;

wxDEFINE_EVENT(FLY_THROUGH_MODE_TOGGLE_EVENT, wxCommandEvent);

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
    m_renderer.screenSpaceZoom(p.x, p.y, m_zoomPerFrame);
  }

  refresh();
}

void Canvas::onResize(wxSizeEvent& e) {
  e.Skip();

  resize();
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

  wxCommandEvent event(FLY_THROUGH_MODE_TOGGLE_EVENT);
  event.SetInt(TOGGLED_ON);
  wxPostEvent(this, event);
}

void Canvas::deactivateFlyThroughMode() {
  m_timer->Stop();
  m_flyThroughMode = false;

  wxCommandEvent event(FLY_THROUGH_MODE_TOGGLE_EVENT);
  event.SetInt(TOGGLED_OFF);
  wxPostEvent(this, event);
}

void Canvas::onLeftMouseBtnDown(wxMouseEvent& e) {
  e.Skip();

  SetFocus();
  SetCurrent(*m_context);

  m_mouseDown = true;
  m_selectionRect = wxRect(e.GetPosition(), wxSize(0, 0));
}

void Canvas::onLeftMouseBtnUp(wxMouseEvent& e) {
  e.Skip();

  m_mouseDown = false;

  if (selectionSizeAboveThreshold(m_selectionRect.GetSize())) {
    int x0 = m_selectionRect.x;
    int y0 = m_selectionRect.y;
    int x1 = x0 + m_selectionRect.width;
    int y1 = y0 + m_selectionRect.height;

    m_renderer.screenSpaceZoom(x0, y0, x1, y1);
  }

  m_renderer.drawSelectionRect(0, 0, 0, 0);
  refresh();
}

void Canvas::onMouseMove(wxMouseEvent& e) {
  e.Skip();

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
  e.Skip();

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
  else if (key == 'I') {
    auto sz = GetClientSize();
    m_renderer.screenSpaceZoom(sz.x / 2, sz.y / 2, m_zoomAmount);
    refresh();
  }
  else if (key == 'O') {
    auto sz = GetClientSize();
    m_renderer.screenSpaceZoom(sz.x / 2, sz.y / 2, 1.0 / m_zoomAmount);
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

void Canvas::refresh() {
  Refresh(false);
}

void Canvas::onPaint(wxPaintEvent&) {
  wxPaintDC dc(this);
  render(dc);
}

void Canvas::render(wxDC&) {
  if (!IsShownOnScreen()) {
    return;
  }

  SetCurrent(*m_context);

  if (!m_renderer.isInitialised()) {
    auto sz = GetSize();
    m_renderer.initialise(sz.x, sz.y);

    PostSizeEvent();
  }

  m_renderer.clear(255, 255, 255);

  if (m_disabled) {
    m_renderer.finish();
    SwapBuffers();

    return;
  }

  m_renderer.drawMandelbrot(m_mouseDown);

  if (m_mouseDown) {
    int x = m_selectionRect.x;
    int y = m_selectionRect.y;
    int w = m_selectionRect.width;
    int h = m_selectionRect.height;

    m_renderer.drawSelectionRect(x, y, w, h);
  }

  m_renderer.finish();
  SwapBuffers();

  measureFrameRate();
  m_onRender();
}

void Canvas::disable() {
  m_disabled = true;
  refresh();
  Disable();
}

void Canvas::enable() {
  m_disabled = false;
  Enable();
}
