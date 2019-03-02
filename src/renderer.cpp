#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include "renderer.hpp"
#include "exception.hpp"

#define INIT_GUARD \
  if (!m_initialised) { \
    return; \
  }

using std::string;
using std::vector;

Renderer::Renderer(int w, int h)
  : brot(w, h) {}

void Renderer::resize(int w, int h) {
  INIT_GUARD

  brot.resize(w, h);
}

void Renderer::initialise() {
  glewExperimental = GL_TRUE;
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    EXCEPTION("Failed to initialize GLEW: " << glewGetErrorString(result));
  }

  if (!glewIsSupported(GL_VERSION_STRING)) {
    GL_SUPPORT_EXCEPTION("OpenGL " << GL_VERSION_MAJOR << "."
                         << GL_VERSION_MINOR << " not supported");
  }

  GL_CHECK(glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE));
  GL_CHECK(glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE));
  GL_CHECK(glPixelStorei(GL_PACK_ROW_LENGTH, 0));
  GL_CHECK(glPixelStorei(GL_PACK_SKIP_ROWS, 0));
  GL_CHECK(glPixelStorei(GL_PACK_SKIP_PIXELS, 0));
  GL_CHECK(glPixelStorei(GL_PACK_ALIGNMENT, 1));

  GL_CHECK(glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE));
  GL_CHECK(glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE));
  GL_CHECK(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
  GL_CHECK(glPixelStorei(GL_UNPACK_SKIP_ROWS, 0));
  GL_CHECK(glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0));
  GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

  brot.initialise();

  m_initialised = true;
}

bool Renderer::isInitialised() const {
  return m_initialised;
}

void Renderer::drawSelectionRect(double x0, double y0, double x1, double y1) {
  INIT_GUARD

  // TODO
}
