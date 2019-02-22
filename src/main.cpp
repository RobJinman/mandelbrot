#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "config.hpp"
#include "mandelbrot.hpp"

const int W = 600;
const int H = 600;

GLFWwindow* setupGlWindow() {
  std::cout << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
    << Mandelbrot_VERSION_MINOR << std::endl;

  glewExperimental = true;

  if (!glfwInit()) {
    std::cerr << "Failed to initialise GLFW" << std::endl;
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = nullptr;
  window = glfwCreateWindow(W, H, "Mandelbrot", NULL, NULL);

  if (window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  return window;
}

void dampenCursorPos(double& x, double& y) {
  double centreX = 0.5 * W;
  double centreY = 0.5 * H;
  double dx = x - centreX;
  double dy = y - centreY;

  double d = sqrt(pow(dx, 2) + pow(dy, 2));
  double sf = 0.5 * d / W;

  x = centreX + dx * sf;
  y = centreY + dy * sf;
}

int main() {
  GLFWwindow* window = setupGlWindow();
  Mandelbrot mandelbrot(W, H);

  glfwSetCursorPos(window, 0.5 * W, 0.5 * H);

  do {
    double x = 0;
    double y = 0;
    glfwGetCursorPos(window, &x, &y);
    dampenCursorPos(x, y);

    mandelbrot.zoom(x, y, 1.05);
    mandelbrot.draw();
    glfwSwapBuffers(window);
    glfwPollEvents();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  return 0;
}
