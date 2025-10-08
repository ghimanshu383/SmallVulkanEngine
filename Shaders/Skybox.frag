#version 450

layout (location = 0) out vec4 color;
layout (location = 0) in vec3 vPos;

layout (set = 0, binding = 0) uniform samplerCube cubeTexture;
void main() {
    color = texture(cubeTexture, normalize(vPos));
}