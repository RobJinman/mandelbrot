#include "mandelbrot.hpp"
#include "exception.hpp"
#include "render_utils.hpp"
#include "utils.hpp"
#include "defaults.hpp"

#define INIT_GUARD \
  if (!m_initialised) { \
    return; \
  }

#define INIT_EXCEPT \
  if (!m_initialised) { \
    throw std::runtime_error("Mandelbrot not initialised"); \
  }

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using std::string;

const std::map<string, string> PRESETS = {
  {
    "Monochrome",

    "float c = float(i) / maxI;\n"
    "return vec3(c, c, c);\n"
  },
  {
    "Colour",

    "float c = float(i) / maxI;\n"
    "return (1.0 - c) * hueToRgb(c);\n"
  },
  {
    "Smooth Colour",

    "float r = sqrt(lastZ.x * lastZ.x + lastZ.y * lastZ.y);\n"
    "if (i < maxI) {\n"
    "  float f = i - log2(log(r));\n"
    "  return hueToRgb(0.8 + f / maxI);\n"
    "}\n"
    "else {\n"
    "  return vec3(0.0, 0.0, 0.0);\n"
    "}\n"
  },
  {
    "Jazzy",

    "float c = float(i) / maxI;\n"
    "return (1.0 - c) * hueToRgb(c * 10.0);\n"
  }
};

static const string COMPUTE_COLOUR_IMPL_SEARCH_STRING = "COMPUTE_COLOUR_IMPL";

static const double INITIAL_XMIN = -2.5;
static const double INITIAL_XMAX = 1.5;
static const double INITIAL_YMIN = -2.0;
static const double INITIAL_YMAX = 2.0;

OfflineRenderStatus::OfflineRenderStatus(int w, int h, int stripH)
  : w(w),
    h(h),
    progress(0),
    data(nullptr),
    stripsDrawn(0),
    stripH(stripH) {

  totalStrips = h / stripH;
  finalStripH = stripH + (h % stripH);
}

Mandelbrot::Mandelbrot() {
  m_renderParams.w = 100;
  m_renderParams.h = 100;
  m_mandelbrotVertShaderPath = "data/mandelbrot_vert_shader.glsl";
  m_mandelbrotFragShaderPath = "data/mandelbrot_frag_shader.glsl";
  m_texVertShaderPath = "data/textured_vert_shader.glsl";
  m_texFragShaderPath = "data/textured_frag_shader.glsl";
}

void Mandelbrot::initialise(int w, int h) {
  GL_CHECK(glGenVertexArrays(1, &m_vao));
  GL_CHECK(glBindVertexArray(m_vao));

  static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // A
    1.0f, -1.0f, 0.0f,    1.0f, 0.0f, // B
    1.0f, 1.0f, 0.0f,     1.0f, 1.0f, // C
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // A
    1.0f, 1.0f, 0.0f,     1.0f, 1.0f, // C
    -1.0f, 1.0f, 0.0f,    0.0f, 1.0f  // D
  };

  GL_CHECK(glGenBuffers(1, &m_vbo));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData),
                        vertexBufferData, GL_STATIC_DRAW));

  m_texProgram = compileProgram(m_texVertShaderPath, m_texFragShaderPath);

  string computeColourImpl = PRESETS.at(DEFAULT_COLOUR_SCHEME);
  compileProgram_(computeColourImpl);
  m_activeComputeColourImpl = computeColourImpl;

  m_renderParams.maxIterations = DEFAULT_MAX_ITERATIONS;
  m_renderParams.xmin = INITIAL_XMIN;
  m_renderParams.xmax = INITIAL_XMAX;
  m_renderParams.ymin = INITIAL_YMIN;
  m_renderParams.ymax = INITIAL_YMAX;

  initUniforms();

  m_initialised = true;

  resize(w, h);
}

void Mandelbrot::reset() {
  m_renderParams.xmin = INITIAL_XMIN;
  m_renderParams.ymin = INITIAL_YMIN;
  m_renderParams.ymax = INITIAL_YMAX;

  double aspect = static_cast<double>(m_renderParams.w) /
                  static_cast<double>(m_renderParams.h);
  m_renderParams.xmax = m_renderParams.xmin + aspect *
                        (m_renderParams.ymax - m_renderParams.ymin);
}

void Mandelbrot::resize(int w, int h) {
  double yScale = static_cast<double>(h) /
                  static_cast<double>(m_renderParams.h);
  double expectedW = m_renderParams.w * yScale;
  double xScale = w / expectedW;
  double xRange = m_renderParams.xmax - m_renderParams.xmin;
  m_renderParams.xmax = m_renderParams.xmin + xRange * xScale;

  m_renderParams.w = w;
  m_renderParams.h = h;
}

GLuint Mandelbrot::renderToTexture(int w, int h) {
  GLuint frameBufferName = 0;
  GL_CHECK(glGenFramebuffers(1, &frameBufferName));
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName));

  GLuint texture;

  GL_CHECK(glGenTextures(1, &texture));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));

  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                        GL_UNSIGNED_BYTE, nullptr));

  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

  GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture,
                                0));

  GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
  GL_CHECK(glDrawBuffers(1, drawBuffers));

  auto status = GL_CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    GL_EXCEPTION("Error creating render target", status);
  }

  render();

  GL_CHECK(glDeleteFramebuffers(1, &frameBufferName));

  return texture;
}

void Mandelbrot::renderStripToMainMemoryBuffer(uint8_t* buffer) {
  auto& rp = m_renderParams;
  auto& rpb = m_renderParamsBackup;
  auto& s = m_offlineRenderStatus;
  int i = s.stripsDrawn;

  int stripH = i < s.totalStrips - 1 ? s.stripH : s.finalStripH;
  double stripH_gph = (rpb.ymax - rpb.ymin) *
                      (static_cast<double>(stripH) / static_cast<double>(s.h));

  GL_CHECK(glUseProgram(m_program.id));

  rp.h = stripH;
  rp.ymax = rp.ymin + stripH_gph;
  GL_CHECK(glViewport(0, 0, s.w, stripH));

  GLuint texture = renderToTexture(s.w, stripH);

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
  GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer));

  rp.ymin += stripH_gph;
}

const OfflineRenderStatus& Mandelbrot::continueOfflineRender() {
  auto& s = m_offlineRenderStatus;

  size_t stride = s.stripH * s.w * 3;
  renderStripToMainMemoryBuffer(s.data + s.stripsDrawn * stride);
  s.stripsDrawn++;

  s.progress = floor(100.0 * static_cast<float>(s.stripsDrawn) /
                     static_cast<float>(s.totalStrips));

  if (s.stripsDrawn == s.totalStrips) {
    s.progress = 100;
    m_renderParams = m_renderParamsBackup;
  }

  return m_offlineRenderStatus;
}

void Mandelbrot::renderToMainMemoryBuffer(int w, int h) {
  INIT_EXCEPT

  int renderStripH = std::min(h, 50);

  m_renderParamsBackup = m_renderParams;
  m_offlineRenderStatus = OfflineRenderStatus(w, h, renderStripH);

  size_t bytes = w * h * 3;
  m_offlineRenderStatus.data = new uint8_t[bytes];

  m_renderParams.w = w;
}

void Mandelbrot::initUniforms() {
  GL_CHECK(glUseProgram(m_program.id));

  m_program.u.w = GL_CHECK(glGetUniformLocation(m_program.id, "u_w"));
  m_program.u.h = GL_CHECK(glGetUniformLocation(m_program.id, "u_h"));
  m_program.u.maxIterations = GL_CHECK(glGetUniformLocation(m_program.id,
                                                            "u_maxIterations"));
  m_program.u.xmin = GL_CHECK(glGetUniformLocation(m_program.id, "u_xmin"));
  m_program.u.xmax = GL_CHECK(glGetUniformLocation(m_program.id, "u_xmax"));
  m_program.u.ymin = GL_CHECK(glGetUniformLocation(m_program.id, "u_ymin"));
  m_program.u.ymax = GL_CHECK(glGetUniformLocation(m_program.id, "u_ymax"));

  updateUniforms();
}

void Mandelbrot::setColourSchemeImpl(const string& computeColourImpl) {
  INIT_GUARD

  try {
    GL_CHECK(glDeleteProgram(m_program.id));
    compileProgram_(computeColourImpl);
  }
  catch (const ShaderException&) {
    compileProgram_(m_activeComputeColourImpl);
    throw;
  }
}

void Mandelbrot::compileProgram_(const std::string& computeColourImpl) {
  m_program.id = compileProgram(m_mandelbrotVertShaderPath,
                                m_mandelbrotFragShaderPath,
                                COMPUTE_COLOUR_IMPL_SEARCH_STRING,
                                computeColourImpl);
  m_activeComputeColourImpl = computeColourImpl;
}

void Mandelbrot::setColourScheme(const string& presetName) {
  INIT_GUARD

  setColourSchemeImpl(PRESETS.at(presetName));
}

void Mandelbrot::updateUniforms() {
  GL_CHECK(glUseProgram(m_program.id));

  auto& rp = m_renderParams;

  GL_CHECK(glUniform1f(m_program.u.w, rp.w));
  GL_CHECK(glUniform1f(m_program.u.h, rp.h));
  GL_CHECK(glUniform1i(m_program.u.maxIterations, rp.maxIterations));
  GL_CHECK(glUniform1f(m_program.u.xmin, rp.xmin));
  GL_CHECK(glUniform1f(m_program.u.xmax, rp.xmax));
  GL_CHECK(glUniform1f(m_program.u.ymin, rp.ymin));
  GL_CHECK(glUniform1f(m_program.u.ymax, rp.ymax));
}

void Mandelbrot::setMaxIterations(int maxI) {
  m_renderParams.maxIterations = maxI;
}

void Mandelbrot::graphSpaceZoom(double x, double y, double mag) {
  auto& rp = m_renderParams;

  double xRange = rp.xmax - rp.xmin;
  double yRange = rp.ymax - rp.ymin;

  double xRangeNew = xRange / mag;
  double yRangeNew = yRange / mag;

  rp.xmin = x - 0.5 * xRangeNew;
  rp.xmax = x + 0.5 * xRangeNew;
  rp.ymin = y - 0.5 * yRangeNew;
  rp.ymax = y + 0.5 * yRangeNew;
}

void Mandelbrot::screenSpaceZoom(double x, double y, double mag) {
  auto& rp = m_renderParams;

  y = rp.h - 1 - y;

  double xRange = rp.xmax - rp.xmin;
  double yRange = rp.ymax - rp.ymin;

  double centreX = rp.xmin + xRange * x / rp.w;
  double centreY = rp.ymin + yRange * y / rp.h;

  graphSpaceZoom(centreX, centreY, mag);
}

void Mandelbrot::screenSpaceZoom(double x0, double y0, double x1, double y1) {
  auto& rp = m_renderParams;

  // Flip and swap
  y0 = rp.h - 1 - y0;
  y1 = rp.h - 1 - y1;
  std::swap(y0, y1);

  double xRange = rp.xmax - rp.xmin;
  double yRange = rp.ymax - rp.ymin;

  double xmin = rp.xmin + (x0 / rp.w) * xRange;
  double xmax = rp.xmin + (x1 / rp.w) * xRange;
  double ymin = rp.ymin + (y0 / rp.h) * yRange;
  double ymax = rp.ymin + (y1 / rp.h) * yRange;

  rp.xmin = xmin;
  rp.xmax = xmax;
  rp.ymin = ymin;
  rp.ymax = ymax;
}

void Mandelbrot::drawFromTexture() {
  GL_CHECK(glUseProgram(m_texProgram));

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_texture));

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

  GL_CHECK(glEnableVertexAttribArray(0));
  GL_CHECK(glEnableVertexAttribArray(1));

  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, BUFFER_OFFSET(0)));
  GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, BUFFER_OFFSET(sizeof(GLfloat) * 3)));

  GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

  GL_CHECK(glDisableVertexAttribArray(0));
  GL_CHECK(glDisableVertexAttribArray(1));
}

void Mandelbrot::draw(bool fromTexture) {
  INIT_GUARD

  if (!fromTexture) {
    GL_CHECK(glDeleteTextures(1, &m_texture));
    m_texture = renderToTexture(m_renderParams.w, m_renderParams.h);
  }

  drawFromTexture();
}

void Mandelbrot::render() {
  GL_CHECK(glUseProgram(m_program.id));
  GL_CHECK(glViewport(0, 0, m_renderParams.w, m_renderParams.h));
  updateUniforms();

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

  GL_CHECK(glEnableVertexAttribArray(0));

  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, BUFFER_OFFSET(0)));

  GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

  GL_CHECK(glDisableVertexAttribArray(0));
}

double Mandelbrot::computeMagnification() const {
  return (INITIAL_YMAX - INITIAL_YMIN) /
         (m_renderParams.ymax - m_renderParams.ymin);
}

double Mandelbrot::getXMin() const {
  return m_renderParams.xmin;
}

double Mandelbrot::getXMax() const {
  return m_renderParams.xmax;
}

double Mandelbrot::getYMin() const {
  return m_renderParams.ymin;
}

double Mandelbrot::getYMax() const {
  return m_renderParams.ymax;
}

int Mandelbrot::getMaxIterations() const {
  return m_renderParams.maxIterations;
}
