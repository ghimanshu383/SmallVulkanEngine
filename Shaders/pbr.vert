#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 normals;

layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} viewProjection;

layout (push_constant) uniform Model {
    mat4 model;
} model;
void main() {
    gl_Position = viewProjection.projection * viewProjection.view * model.model * vec4(pos, 1.0);
}