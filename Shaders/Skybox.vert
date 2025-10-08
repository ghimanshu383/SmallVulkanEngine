#version 450

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 vPos;
layout (push_constant) uniform ViewProjection {
    mat4 viewProjection;
} camera;

void main() {
    vPos = pos;
    vec4 clip = camera.viewProjection * vec4(pos, 1.0);
    gl_Position = vec4(clip.x, clip.y, clip.w, clip.w); // clip.xyww
}