#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex;

out vec2 vv_tex;

void main() {
  gl_Position.xyz = in_pos;
  gl_Position.w = 1.0;

  vv_tex = in_tex;
}
