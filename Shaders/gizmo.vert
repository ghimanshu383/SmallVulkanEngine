#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec4 vColor;
layout (location = 1) out flat uint vId;
layout (location = 2) out flat uint vPickId;
layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} viewProjection;

layout (push_constant) uniform Model {
    mat4 model;
    uint pickId;
} model;
void main() {
    // The model matrix has to be without rotation or scaling.
    gl_Position = viewProjection.projection * viewProjection.view * model.model * vec4(pos, 1.0);
    vColor = color;
    vId = uint(uv.x);
    vPickId = model.pickId;
}