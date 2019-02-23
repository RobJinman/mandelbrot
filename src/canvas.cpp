#include "canvas.hpp"
#include "mandelbrot.hpp"

wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
  EVT_PAINT(Canvas::render)
  EVT_SIZE(Canvas::resized)
wxEND_EVENT_TABLE()

Canvas::Canvas(wxFrame* parent, const int* args, Mandelbrot& mandelbrot)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition,
               wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    m_mandelbrot(mandelbrot) {

  wxGLContextAttrs attrs;
  attrs.MajorVersion(3).MinorVersion(3).ForwardCompatible();
  m_context.reset(new wxGLContext(this, nullptr, &attrs));

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

void Canvas::resized(wxSizeEvent& e) {
  auto sz = e.GetSize();
  m_mandelbrot.resize(sz.x, sz.y);

  Refresh();
}

void Canvas::initGl() {
  wxGLCanvas::SetCurrent(*m_context);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  m_mandelbrot.init();

  m_initialised = true;
}

void Canvas::render(wxPaintEvent& e) {
  if (!IsShown()) {
    return;
  }

  if (!m_initialised) {
    initGl();
  }

  wxGLCanvas::SetCurrent(*m_context);
  wxPaintDC(this);

  m_mandelbrot.draw();

  glFlush();
  SwapBuffers();
}
