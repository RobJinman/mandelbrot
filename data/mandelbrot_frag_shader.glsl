#version 330 core

precision highp float;
precision highp int;

const float RADIUS = 10000.0;

uniform float u_w;
uniform float u_h;
uniform int u_maxIterations;
uniform float u_xmin;
uniform float u_xmax;
uniform float u_ymin;
uniform float u_ymax;

layout(location = 0) out vec3 out_colour;

struct Result {
  int i;
  vec2 zn;
};

Result testPoint(vec2 p) {
  float x0 = p.x;
  float y0 = p.y;
  float x = x0;
  float y = y0;

  int i = 0;
  for (; i < u_maxIterations; ++i) {
    float nextX = x * x - y * y + x0;
    float nextY = 2.0 * x * y + y0;

    x = nextX;
    y = nextY;

    if (x * x + y * y > RADIUS) {
      break;
    }
  }

  return Result(i, vec2(x, y));
}

vec2 screenToWorld(vec2 p) {
  float w = u_xmax - u_xmin;
  float h = u_ymax - u_ymin;

  return vec2(u_xmin + w * p.x / u_w, u_ymin + h * p.y / u_h);
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

vec3 computeColour(int i, int maxI, vec2 lastZ) {
COMPUTE_COLOUR_IMPL
}

void main() {
  vec2 p = screenToWorld(gl_FragCoord.xy);
  Result res = testPoint(p);
  out_colour = vec3(computeColour(res.i, u_maxIterations, res.zn));
}
