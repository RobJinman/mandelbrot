#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "config.hpp"


int main() {
  std::cout << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
    << Mandelbrot_VERSION_MINOR << std::endl;

  glewExperimental = true;

  if (!glfwInit()) {
    std::cerr << "Failed to initialise GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = nullptr;
  window = glfwCreateWindow(1024, 768, "Mandelbrot", NULL, NULL);

  if (window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return 1;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  do {
      glClear(GL_COLOR_BUFFER_BIT);

      glfwSwapBuffers(window);
      glfwPollEvents();
  }
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  return 0;
}

