#version 450

vec2 vert[3] = {
vec2(0, -.5),
vec2(-0.5, 0),
vec2(0.5, 0)
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

layout (location  = 0 ) out vec4 vColor;
void main() {
    gl_Position = vec4(pos, 1.f);
    vColor = color;
}