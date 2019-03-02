#version 330 core

uniform u_colour;

layout(location = 0) out vec3 out_colour;

void main() {
  out_colour = u_colour;
}
