#version 450

vec2 vert[3] = {
vec2(0, -.5),
vec2(-0.5, 0),
vec2(0.5, 0)
};

void main() {
    gl_Position = vec4(vert[gl_VertexIndex], 0, 1.f);
}