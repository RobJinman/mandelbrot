#version 330 core

uniform float W;
uniform float H;

out vec3 color;

int maxIterations = 100;
float xmin = -2.0;
float xmax = 2.0;
float ymin = -2.0;
float ymax = 2.0;

int testPoint(vec2 p) {
  float x0 = p.x;
  float y0 = p.y;
  float x = x0;
  float y = y0;

  int i = 0;
  for (; i < maxIterations; ++i) {
    float nextX = x * x - y * y + x0;
    float nextY = 2.0 * x * y + y0;

    x = nextX;
    y = nextY;

    if (x * x + y * y > 4.0) {
      break;
    }
  }

  return i;
}

vec2 screenToWorld(vec2 p) {
  float w = xmax - xmin;
  float h = ymax - ymin;

  return vec2(xmin + w * p.x / W, ymin + h * p.y / H);
}

vec3 getColour(int i) {
  float c = i / maxIterations;
  return vec3(c, c, c);
}

void main() {
  int i = testPoint(screenToWorld(gl_FragCoord.xy));
  color = vec3(getColour(i));
}

