#version 330 core

uniform sampler2D texture;

in vec2 vv_tex;

layout(location = 0) out vec3 out_colour;

void main() {
  out_colour = texture2D(texture, vv_tex).xyz;
}
