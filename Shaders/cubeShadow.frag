#version 450

layout (location = 0) in vec3 vWorldPos;
layout (set = 0, binding = 1) uniform LightData {
    vec4 position;
    float farPlane;
    float _padding[3];
} lightData;

layout (location = 0) out float depth;

void main() {
    float distance = length(lightData.position.xyz -vWorldPos );
    depth = distance / 100;
}