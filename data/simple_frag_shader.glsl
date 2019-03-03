#version 330 core

uniform vec3 u_colour;

layout(location = 0) out vec3 out_colour;

void main() {
  out_colour = u_colour;
}
