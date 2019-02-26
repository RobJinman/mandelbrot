#version 330 core

precision highp float;
precision highp int;

uniform float W;
uniform float H;
uniform int maxIterations;
uniform float xmin;
uniform float xmax;
uniform float ymin;
uniform float ymax;

layout(location = 0) out vec3 color;

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

vec3 hueToRgb(float hue) {
  float h = mod(hue, 1.0) * 6.0;
  float x = 1.0 - abs(mod(h, 2) - 1.0);
  if (h < 1.0) {
    return vec3(1.0, x, 0.0);
  }
  else if (h < 2.0) {
    return vec3(x, 1.0, 0.0);
  }
  else if (h < 3.0) {
    return vec3(0.0, 1.0, x);
  }
  else if (h < 4.0) {
    return vec3(0.0, x, 1.0);
  }
  else if (h < 5.0) {
    return vec3(x, 0.0, 1.0);
  }
  else {
    return vec3(1.0, 0.0, x);
  }
}

vec3 computeColour(int i, float x, float y) {
COMPUTE_COLOUR_IMPL
}

void main() {
  vec2 p = screenToWorld(gl_FragCoord.xy);
  int i = testPoint(p);
  color = vec3(computeColour(i, p.x, p.y));
}
