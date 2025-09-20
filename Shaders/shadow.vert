#version 450

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 vPos;
layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} viewProjection;

layout (push_constant) uniform Model {
    mat4 model;
} model;
void main() {
    gl_Position = viewProjection.projection * viewProjection.view * model.model * vec4(pos, 1);
    vPos = pos;
}