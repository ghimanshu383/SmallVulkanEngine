#version 450

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 vWorldPos;

layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} viewProjection;

layout (push_constant) uniform Model {
    mat4 model;
} model;
void main() {
    vec4 worldPos = model.model * vec4(pos, 1.0);
    vWorldPos = worldPos.xyz;
    gl_Position = viewProjection.projection * viewProjection.view * worldPos;
}