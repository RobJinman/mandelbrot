#include "renderer.hpp"
#include "exception.hpp"
#include "render_utils.hpp"

#define INIT_GUARD \
  if (!m_initialised) { \
    return; \
  }

using std::string;

static const int FLOATS_PER_VERT = 3;
static const int VERTS_PER_RECT = 6;
static const int FLOATS_PER_RECT = VERTS_PER_RECT * FLOATS_PER_VERT;

static const GLfloat COLOUR[3] = { 0.0f, 1.0f, 0.0f };

Renderer::Renderer(int w, int h, std::function<void()> fnMakeGlContextCurrent)
  : m_brot(w, h),
    m_fnMakeGlContextCurrent(fnMakeGlContextCurrent) {

  m_w = w;
  m_h = h;

  m_vertShaderPath = "data/simple_vert_shader.glsl";
  m_fragShaderPath = "data/simple_frag_shader.glsl";
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

  m_brot.initialise();

  m_program.id = compileProgram(m_vertShaderPath, m_fragShaderPath);
  initUniforms();

  GL_CHECK(glGenVertexArrays(1, &m_program.vao));
  GL_CHECK(glBindVertexArray(m_program.vao));

  GL_CHECK(glGenBuffers(1, &m_program.vbo));

  GLfloat vertexBufferData[4 * FLOATS_PER_RECT];
  memset(vertexBufferData, 0, sizeof(vertexBufferData));

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_program.vbo));
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData),
                        vertexBufferData, GL_DYNAMIC_DRAW));

  m_initialised = true;
}

void Renderer::initUniforms() { 
  GL_CHECK(glUseProgram(m_program.id));

  m_program.u.colour = GL_CHECK(glGetUniformLocation(m_program.id, "u_colour"));

  updateUniforms(COLOUR);
}

void Renderer::draw(bool fromTexture) {
  INIT_GUARD
  m_fnMakeGlContextCurrent();

  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  m_brot.draw(fromTexture);

  GL_CHECK(glUseProgram(m_program.id));

  GL_CHECK(glEnableVertexAttribArray(0));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_program.vbo));
  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));

  GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 4 * VERTS_PER_RECT));
  GL_CHECK(glDisableVertexAttribArray(0));

  GL_CHECK(glFlush());
}

void Renderer::resize(int w, int h) {
  INIT_GUARD
  m_fnMakeGlContextCurrent();

  m_brot.resize(w, h);
  m_w = w;
  m_h = h;
}

void Renderer::updateUniforms(const float colour[3]) {
  m_fnMakeGlContextCurrent();

  GL_CHECK(glUseProgram(m_program.id));
  GL_CHECK(glUniform3f(m_program.u.colour, colour[0], colour[1], colour[2]));
}

bool Renderer::isInitialised() const {
  return m_initialised;
}

double Renderer::screenToClipSpaceX(double x) const {
  return 2.0 * x / m_w - 1.0;
}

double Renderer::screenToClipSpaceY(double y) const {
  return 2.0 * (1.0 - y / m_h) - 1.0;
}

void Renderer::makeRectangle(double x, double y, double w, double h,
                             GLfloat* verts) const {

  double x0 = screenToClipSpaceX(x);
  double y0 = screenToClipSpaceY(y);
  double x1 = screenToClipSpaceX(x + w);
  double y1 = screenToClipSpaceY(y + h);

  int v = 0;
  verts[v + 0] = x0;    // A
  verts[v + 1] = y0;
  verts[v + 2] = 0.0f;
  v += 3;
  verts[v + 0] = x1;    // B
  verts[v + 1] = y0;
  verts[v + 2] = 0.0f;
  v += 3;
  verts[v + 0] = x1;    // C
  verts[v + 1] = y1;
  verts[v + 2] = 0.0f;
  v += 3;
  verts[v + 0] = x0;    // A
  verts[v + 1] = y0;
  verts[v + 2] = 0.0f;
  v += 3;
  verts[v + 0] = x1;    // C
  verts[v + 1] = y1;
  verts[v + 2] = 0.0f;
  v += 3;
  verts[v + 0] = x0;    // D
  verts[v + 1] = y1;
  verts[v + 2] = 0.0f;
}

void Renderer::makeSelectionRect(double x, double y, double w, double h) {
  GLfloat d = 2.0f;

  GLfloat verts[4 * FLOATS_PER_RECT];
  memset(verts, 0, sizeof(verts));

  makeRectangle(x, y, d, h, verts + FLOATS_PER_RECT * 0);
  makeRectangle(x + d, y, w - d, d, verts + FLOATS_PER_RECT * 1);
  makeRectangle(x + w - d, y + d, d, h - d, verts + FLOATS_PER_RECT * 2);
  makeRectangle(x, y + h - d, w - d, d, verts + FLOATS_PER_RECT * 3);

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_program.vbo));
  GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0,
                           sizeof(GLfloat) * 4 * FLOATS_PER_RECT, verts));
}

void Renderer::drawSelectionRect(double x, double y, double w, double h) {
  INIT_GUARD
  m_fnMakeGlContextCurrent();

  makeSelectionRect(x, y, w, h);

  updateUniforms(COLOUR);
}

void Renderer::zoom(double x, double y, double mag) {
  m_fnMakeGlContextCurrent();
  m_brot.zoom(x, y, mag);
}

void Renderer::zoom(double x0, double y0, double x1, double y1) {
  m_fnMakeGlContextCurrent();
  m_brot.zoom(x0, y0, x1, y1);
}

void Renderer::resetZoom() {
  m_fnMakeGlContextCurrent();
  m_brot.reset();
}

void Renderer::setMaxIterations(int maxI) {
  m_fnMakeGlContextCurrent();
  m_brot.setMaxIterations(maxI);
}

void Renderer::setColourScheme(const std::string& presetName) {
  m_fnMakeGlContextCurrent();
  m_brot.setColourScheme(presetName);
}

void Renderer::setColourSchemeImpl(const std::string& computeColourImpl) {
  m_fnMakeGlContextCurrent();
  m_brot.setColourSchemeImpl(computeColourImpl);
}

double Renderer::getXMin() const {
  return m_brot.getXMin();
}

double Renderer::getXMax() const {
  return m_brot.getXMax();
}

double Renderer::getYMin() const {
  return m_brot.getYMin();
}

double Renderer::getYMax() const {
  return m_brot.getYMax();
}

int Renderer::getMaxIterations() const {
  return m_brot.getMaxIterations();
}

double Renderer::computeMagnification() const {
  return m_brot.computeMagnification();
}

uint8_t* Renderer::renderToMainMemoryBuffer(int w, int h, size_t& bytes) {
  m_fnMakeGlContextCurrent();
  return m_brot.renderToMainMemoryBuffer(w, h, bytes);
}
