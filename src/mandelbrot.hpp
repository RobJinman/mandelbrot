#pragma once

#include <string>
#include <map>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <future>
#include "gl.hpp"

extern const std::map<std::string, std::string> PRESETS;
const int DEFAULT_MAX_ITERATIONS = 100;
const std::string DEFAULT_COLOUR_SCHEME = "coloured";

class Mandelbrot {
public:
  Mandelbrot(int W, int H);

  void init(const std::function<void()>& fnCreateGlContext);
  void resize(int w, int y);
  void draw();
  void zoom(double x, double y, double mag);
  void zoom(double x0, double y0, double x1, double y1);
  void reset();

  void setMaxIterations(int maxI);
  void setColourScheme(const std::string& presetName);
  void setColourSchemeImpl(const std::string& computeColourImpl);

  double getXMin() const;
  double getXMax() const;
  double getYMin() const;
  double getYMax() const;
  int getMaxIterations() const;

  double computeMagnification() const;

  std::future<uint8_t*> renderToMainMemoryBuffer(int w, int h, size_t& bytes);

  ~Mandelbrot();

private:
  std::atomic<bool> m_alive;
  std::thread m_thread;
  std::mutex m_cvMutex;
  std::condition_variable m_cv;
  std::string m_pendingFunctionCall;
  std::atomic<bool> m_busy;

  void notifyThatFunctionIsPending(const std::string& name);
  void awaitResult();

  // All functions with a trailing underscore run on worker thread
  //
  void loop_();
  void doDispatch_();

  struct {
    std::function<void()> fnCreateGlContext;
  } m_initArgs;

  void init_(const std::function<void()>& createGlContext);
  void draw_();

  struct {
    std::string vertShaderPath;
    std::string fragShaderPath;
    std::string computeColourImpl;
  } m_loadShadersArgs;
  
  void loadShaders_(const std::string& fragShaderPath,
                    const std::string& vertShaderPath,
                    const std::string& computeColourImpl);
  GLuint loadShader_(const std::string& path, GLuint type,
                     const std::string& computeColourImpl = "");
  void initUniforms_();
  void updateUniforms_();

  struct {
    int w;
    int h;
    size_t* bytes = nullptr;
    uint8_t* result = nullptr;
  } m_renderToMainMemoryBufferArgs;

  uint8_t* renderToMainMemoryBuffer_(int w, int h, size_t& bytes);

  struct {
    int w;
    int h;
  } m_resizeArgs;

  void resize_(int w, int h);

  std::atomic<bool> m_initialised;
  std::atomic<int> m_W;
  std::atomic<int> m_H;
  std::atomic<int> m_maxIterations;
  std::atomic<double> m_xmin;
  std::atomic<double> m_xmax;
  std::atomic<double> m_ymin;
  std::atomic<double> m_ymax;

  std::string m_activeComputeColourImpl;

  struct {
    GLuint uW;
    GLuint uH;
    GLuint uMaxIterations;
    GLuint uXmin;
    GLuint uXmax;
    GLuint uYmin;
    GLuint uYmax;
  } m_uniforms;

  GLuint m_vertexBuffer;
  GLuint m_program = 0;
};
