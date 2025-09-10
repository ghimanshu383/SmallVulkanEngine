#version 450

layout (location = 0) in vec4 vColor;
layout (location = 1) in vec2 textureCoords;
layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D defaultSampler;
void main() {
    color = texture(defaultSampler, textureCoords);
}