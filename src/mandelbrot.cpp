#include "mandelbrot.hpp"
#include "exception.hpp"
#include "render_utils.hpp"

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
    "monochrome",

    "float c = float(i) / float(u_maxIterations);\n"
    "return vec3(c, c, c);\n"
  },
  {
    "coloured",

    "float c = float(i) / float(u_maxIterations);\n"
    "return (1.0 - c) * hueToRgb(c);\n"
  },
  {
    "spacey",

    "float c = float(i) / float(u_maxIterations);\n"
    "return 0.7 * c * c * hueToRgb(c * 0.5 + 0.65);\n"
  },
  {
    "jazzy",

    "float c = float(i) / float(u_maxIterations);\n"
    "return (1.0 - c) * hueToRgb(c * 10.0);\n"
  },
  {
    "rainbow",

    "float c = float(i) / float(u_maxIterations);\n"
    "float r = sqrt(x * x + y * y) / 2.0;\n"
    "return (1.0 - c) * hueToRgb(c + r);\n"
  }
};

static const string COMPUTE_COLOUR_IMPL_SEARCH_STRING = "COMPUTE_COLOUR_IMPL";

static const double INITIAL_XMIN = -2.5;
static const double INITIAL_XMAX = 1.5;
static const double INITIAL_YMIN = -2.0;
static const double INITIAL_YMAX = 2.0;

Mandelbrot::Mandelbrot(int w, int h) {
  m_w = w;
  m_h = h;
  m_mandelbrotVertShaderPath = "data/mandelbrot_vert_shader.glsl";
  m_mandelbrotFragShaderPath = "data/mandelbrot_frag_shader.glsl";
  m_texVertShaderPath = "data/textured_vert_shader.glsl";
  m_texFragShaderPath = "data/textured_frag_shader.glsl";
}

void Mandelbrot::initialise() {
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

  m_maxIterations = DEFAULT_MAX_ITERATIONS;
  m_xmin = INITIAL_XMIN;
  m_xmax = INITIAL_XMAX;
  m_ymin = INITIAL_YMIN;
  m_ymax = INITIAL_YMAX;

  initUniforms();

  m_initialised = true;
}

void Mandelbrot::reset() {
  m_maxIterations = DEFAULT_MAX_ITERATIONS;
  m_xmin = INITIAL_XMIN;
  m_ymin = INITIAL_YMIN;
  m_ymax = INITIAL_YMAX;

  double aspect = static_cast<double>(m_w) / static_cast<double>(m_h);
  m_xmax = m_xmin + aspect * (m_ymax - m_ymin);

  INIT_GUARD

  updateUniforms();
}

void Mandelbrot::resize(int w, int h) {
  double yScale = static_cast<double>(h) / static_cast<double>(m_h);
  double expectedW = m_w * yScale;
  double xScale = w / expectedW;
  double xRange = m_xmax - m_xmin;
  m_xmax = m_xmin + xRange * xScale;

  m_w = w;
  m_h = h;

  INIT_GUARD

  GL_CHECK(glUseProgram(m_program.id));
  GL_CHECK(glViewport(0, 0, w, h));

  updateUniforms();
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

uint8_t* Mandelbrot::renderToMainMemoryBuffer(int w, int h, size_t& bytes) {
  INIT_EXCEPT

  GL_CHECK(glUseProgram(m_program.id));

  int prevW = m_w;
  int prevH = m_h;
  m_w = w;
  m_h = h;
  GL_CHECK(glViewport(0, 0, w, h));
  updateUniforms();

  GLuint frameBufferName = 0;
  GL_CHECK(glGenFramebuffers(1, &frameBufferName));
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName));

  GLuint texture = renderToTexture(w, h);

  bytes = w * h * 3;
  uint8_t* data = new uint8_t[bytes];

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
  GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data));

  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  resize(prevW, prevH);

  return data;
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
    updateUniforms();
  }
  catch (const ShaderException&) {
    compileProgram_(m_activeComputeColourImpl);
    updateUniforms();
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

  GL_CHECK(glUniform1f(m_program.u.w, m_w));
  GL_CHECK(glUniform1f(m_program.u.h, m_h));
  GL_CHECK(glUniform1i(m_program.u.maxIterations, m_maxIterations));
  GL_CHECK(glUniform1f(m_program.u.xmin, m_xmin));
  GL_CHECK(glUniform1f(m_program.u.xmax, m_xmax));
  GL_CHECK(glUniform1f(m_program.u.ymin, m_ymin));
  GL_CHECK(glUniform1f(m_program.u.ymax, m_ymax));
}

void Mandelbrot::setMaxIterations(int maxI) {
  INIT_GUARD

  m_maxIterations = maxI;
  updateUniforms();
}

void Mandelbrot::zoom(double x, double y, double mag) {
  INIT_GUARD

  y = m_h - 1 - y;

  double xRange = m_xmax - m_xmin;
  double yRange = m_ymax - m_ymin;

  double centreX = m_xmin + xRange * x / m_w;
  double centreY = m_ymin + yRange * y / m_h;

  double sf = mag;

  double xRangeNew = xRange / sf;
  double yRangeNew = yRange / sf;

  m_xmin = centreX - 0.5 * xRangeNew;
  m_xmax = centreX + 0.5 * xRangeNew;
  m_ymin = centreY - 0.5 * yRangeNew;
  m_ymax = centreY + 0.5 * yRangeNew;

  updateUniforms();
}

void Mandelbrot::zoom(double x0, double y0, double x1, double y1) {
  INIT_GUARD

  // Flip and swap
  y0 = m_h - 1 - y0;
  y1 = m_h - 1 - y1;
  std::swap(y0, y1);

  double xRange = m_xmax - m_xmin;
  double yRange = m_ymax - m_ymin;

  double xmin = m_xmin + (x0 / m_w) * xRange;
  double xmax = m_xmin + (x1 / m_w) * xRange;
  double ymin = m_ymin + (y0 / m_h) * yRange;
  double ymax = m_ymin + (y1 / m_h) * yRange;

  m_xmin = xmin;
  m_xmax = xmax;
  m_ymin = ymin;
  m_ymax = ymax;

  updateUniforms();
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
    m_texture = renderToTexture(m_w, m_h);
  }

  drawFromTexture();
}

void Mandelbrot::render() {
  GL_CHECK(glUseProgram(m_program.id));

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

  GL_CHECK(glEnableVertexAttribArray(0));

  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, BUFFER_OFFSET(0)));

  GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

  GL_CHECK(glDisableVertexAttribArray(0));
}

double Mandelbrot::computeMagnification() const {
  return (INITIAL_YMAX - INITIAL_YMIN) / (m_ymax - m_ymin);
}

double Mandelbrot::getXMin() const {
  return m_xmin;
}

double Mandelbrot::getXMax() const {
  return m_xmax;
}

double Mandelbrot::getYMin() const {
  return m_ymin;
}

double Mandelbrot::getYMax() const {
  return m_ymax;
}

int Mandelbrot::getMaxIterations() const {
  return m_maxIterations;
}
