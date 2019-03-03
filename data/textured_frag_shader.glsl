#version 330 core

uniform sampler2D texSampler;

in vec2 vv_tex;

layout(location = 0) out vec3 out_colour;

void main() {
  out_colour = texture(texSampler, vv_tex).xyz;
}
