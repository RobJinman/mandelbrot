#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include "mandelbrot.hpp"
#include "exception.hpp"

using std::string;
using std::vector;

const std::map<string, string> PRESETS = {
  {
    "monochrome",

    "float c = float(i) / float(maxIterations);\n"
    "return vec3(c, c, c);\n"
  },
  {
    "coloured",

    "float c = float(i) / float(maxIterations);\n"
    "return (1.0 - c) * hueToRgb(c);\n"
  },
  {
    "spacey",

    "float c = float(i) / float(maxIterations);\n"
    "return 0.7 * c * c * hueToRgb(c * 0.5 + 0.65);\n"
  },
  {
    "jazzy",

    "float c = float(i) / float(maxIterations);\n"
    "return (1.0 - c) * hueToRgb(c * 10.0);\n"
  },
  {
    "rainbow",

    "float c = float(i) / float(maxIterations);\n"
    "float r = sqrt(x * x + y * y) / 2.0;\n"
    "return (1.0 - c) * hueToRgb(c + r);\n"
  }
};

static const string COMPUTE_COLOUR_IMPL_SEARCH_STRING = "COMPUTE_COLOUR_IMPL";

static const double INITIAL_XMIN = -2.5;
static const double INITIAL_XMAX = 1.5;
static const double INITIAL_YMIN = -2.0;
static const double INITIAL_YMAX = 2.0;

Mandelbrot::Mandelbrot(int W, int H) {
  m_W = W;
  m_H = H;
  reset();
}

void Mandelbrot::reset() {
  m_maxIterations = DEFAULT_MAX_ITERATIONS;
  m_xmin = INITIAL_XMIN;
  m_xmax = INITIAL_XMAX;
  m_ymin = INITIAL_YMIN;
  m_ymax = INITIAL_YMAX;

  if (m_initialised) {
    updateUniforms();
  }
}

void Mandelbrot::resize(int w, int h) {
  if (!m_initialised) {
    return;
  }

  double yScale = static_cast<double>(h) / static_cast<double>(m_H);
  double expectedW = m_W * yScale;
  double xScale = w / expectedW;
  double xRange = m_xmax - m_xmin;
  m_xmax = m_xmin + xRange * xScale;

  m_W = w;
  m_H = h;
  GL_CHECK(glViewport(0, 0, w, h));

  updateUniforms();
}

void Mandelbrot::init() {
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

  GLuint vertexArray;
  GL_CHECK(glGenVertexArrays(1, &vertexArray));
  GL_CHECK(glBindVertexArray(vertexArray));

  static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f, // A
    1.0f, -1.0f, 0.0f,  // B
    1.0f, 1.0f, 0.0f,   // C
    -1.0f, -1.0f, 0.0f, // A
    1.0f, 1.0f, 0.0f,   // C
    -1.0f, 1.0f, 0.0f,  // D
  };

  GL_CHECK(glGenBuffers(1, &m_vertexBuffer));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer));
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData),
                        vertexBufferData, GL_STATIC_DRAW));

  loadShaders("data/vert_shader.glsl", "data/frag_shader.glsl",
              PRESETS.at(DEFAULT_COLOUR_SCHEME));

  initUniforms();

  m_initialised = true;
}

uint8_t* Mandelbrot::renderToMainMemoryBuffer(int w, int h, size_t& bytes) {
  if (!m_initialised) {
    EXCEPTION("Mandelbrot not initialised");
  }

  int prevW = m_W;
  int prevH = m_H;
  m_W = w;
  m_H = h;
  GL_CHECK(glViewport(0, 0, w, h));
  updateUniforms();

  GLuint frameBufferName = 0;
  GL_CHECK(glGenFramebuffers(1, &frameBufferName));
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName));

  GLuint texture = 0;
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

  draw();

  bytes = w * h * 3;
  uint8_t* data = new uint8_t[bytes];

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
  GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data));

  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  resize(prevW, prevH);

  return data;
}

void Mandelbrot::initUniforms() {
  m_uniforms.uW = GL_CHECK(glGetUniformLocation(m_program, "W"));
  m_uniforms.uH = GL_CHECK(glGetUniformLocation(m_program, "H"));
  m_uniforms.uMaxIterations = GL_CHECK(glGetUniformLocation(m_program,
                                                            "maxIterations"));
  m_uniforms.uXmin = GL_CHECK(glGetUniformLocation(m_program, "xmin"));
  m_uniforms.uXmax = GL_CHECK(glGetUniformLocation(m_program, "xmax"));
  m_uniforms.uYmin = GL_CHECK(glGetUniformLocation(m_program, "ymin"));
  m_uniforms.uYmax = GL_CHECK(glGetUniformLocation(m_program, "ymax"));

  updateUniforms();
}

GLuint Mandelbrot::loadShader(const string& srcPath, GLuint type,
                              const string& computeColourImpl) {
  GLuint shaderId = GL_CHECK(glCreateShader(type));

  std::ifstream fin(srcPath);
  string shaderSrc{std::istreambuf_iterator<char>(fin),
                   std::istreambuf_iterator<char>()};

  if (type == GL_FRAGMENT_SHADER) {
    auto idx = shaderSrc.find(COMPUTE_COLOUR_IMPL_SEARCH_STRING);
    auto len = COMPUTE_COLOUR_IMPL_SEARCH_STRING.length();
    shaderSrc.replace(idx, len, computeColourImpl);
  }

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  const char* srcPtr = shaderSrc.c_str();
  GL_CHECK(glShaderSource(shaderId, 1, &srcPtr, NULL));
  GL_CHECK(glCompileShader(shaderId));

  GL_CHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
  GL_CHECK(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLen));
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    GL_CHECK(glGetShaderInfoLog(shaderId, infoLogLen, NULL, errMsg.data()));
    throw ShaderException(errMsg.data());
  }

  return shaderId;
}

void Mandelbrot::loadShaders(const string& vertShaderPath,
                             const string& fragShaderPath,
                             const string& computeColourImpl) {
  GLuint vertShader = loadShader(vertShaderPath, GL_VERTEX_SHADER);
  GLuint fragShader = loadShader(fragShaderPath, GL_FRAGMENT_SHADER,
                                 computeColourImpl);

  m_program = GL_CHECK(glCreateProgram());
  GL_CHECK(glAttachShader(m_program, vertShader));
  GL_CHECK(glAttachShader(m_program, fragShader));
  GL_CHECK(glLinkProgram(m_program));

  GLint result = GL_FALSE;
  int infoLogLen = 0;

  GL_CHECK(glGetProgramiv(m_program, GL_LINK_STATUS, &result));
  GL_CHECK(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLen));
  if (infoLogLen > 0) {
    vector<char> errMsg(infoLogLen + 1);
    GL_CHECK(glGetProgramInfoLog(m_program, infoLogLen, NULL, errMsg.data()));
    throw ShaderException(errMsg.data());
  }

  GL_CHECK(glDetachShader(m_program, vertShader));
  GL_CHECK(glDetachShader(m_program, fragShader));

  GL_CHECK(glDeleteShader(vertShader));
  GL_CHECK(glDeleteShader(fragShader));

  GL_CHECK(glUseProgram(m_program));

  m_activeComputeColourImpl = computeColourImpl;
}

void Mandelbrot::setColourSchemeImpl(const string& computeColourImpl) {
  if (!m_initialised) {
    return;
  }

  try {
    GL_CHECK(glDeleteProgram(m_program));
    loadShaders("data/vert_shader.glsl", "data/frag_shader.glsl",
                computeColourImpl);
    updateUniforms();
  }
  catch (const ShaderException&) {
    loadShaders("data/vert_shader.glsl", "data/frag_shader.glsl",
                m_activeComputeColourImpl);

    throw;
  }
}

void Mandelbrot::setColourScheme(const string& presetName) {
  if (!m_initialised) {
    return;
  }

  setColourSchemeImpl(PRESETS.at(presetName));
}

void Mandelbrot::updateUniforms() {
  GL_CHECK(glUniform1f(m_uniforms.uW, m_W));
  GL_CHECK(glUniform1f(m_uniforms.uH, m_H));
  GL_CHECK(glUniform1i(m_uniforms.uMaxIterations, m_maxIterations));
  GL_CHECK(glUniform1f(m_uniforms.uXmin, m_xmin));
  GL_CHECK(glUniform1f(m_uniforms.uXmax, m_xmax));
  GL_CHECK(glUniform1f(m_uniforms.uYmin, m_ymin));
  GL_CHECK(glUniform1f(m_uniforms.uYmax, m_ymax));
}

void Mandelbrot::setMaxIterations(int maxI) {
  if (!m_initialised) {
    return;
  }

  m_maxIterations = maxI;
  updateUniforms();
}

void Mandelbrot::zoom(double x, double y, double mag) {
  if (!m_initialised) {
    return;
  }

  y = m_H - 1 - y;

  double xRange = m_xmax - m_xmin;
  double yRange = m_ymax - m_ymin;

  double centreX = m_xmin + xRange * x / m_W;
  double centreY = m_ymin + yRange * y / m_H;

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
  if (!m_initialised) {
    return;
  }

  // Flip and swap
  y0 = m_H - 1 - y0;
  y1 = m_H - 1 - y1;
  std::swap(y0, y1);

  double xRange = m_xmax - m_xmin;
  double yRange = m_ymax - m_ymin;

  double xmin = m_xmin + (x0 / m_W) * xRange;
  double xmax = m_xmin + (x1 / m_W) * xRange;
  double ymin = m_ymin + (y0 / m_H) * yRange;
  double ymax = m_ymin + (y1 / m_H) * yRange;

  m_xmin = xmin;
  m_xmax = xmax;
  m_ymin = ymin;
  m_ymax = ymax;

  updateUniforms();
}

void Mandelbrot::draw() {
  if (!m_initialised) {
    return;
  }

  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  GL_CHECK(glUseProgram(m_program));

  GL_CHECK(glEnableVertexAttribArray(0));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer));
  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));

  GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
  GL_CHECK(glDisableVertexAttribArray(0));

  GL_CHECK(glFlush());
}

void Mandelbrot::drawSelectionRect(double x0, double y0, double x1, double y1) {
  if (!m_initialised) {
    return;
  }

  // TODO
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
