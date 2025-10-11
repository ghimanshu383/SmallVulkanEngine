#version 450

layout (location = 0) out vec4 color;
layout (set = 0, binding = 1) uniform OmniDirectionalInfo {
    mat4 projection;
    mat4 view;
    vec4 position;
    vec4 color;
    vec4 intensities;
} lightInfo;

struct PointLights {
    vec4 position;
    vec4 color;
    vec4 intensities;
};
const int MAX_POINT_LIGHT_COUNT = 10;
layout (set = 0, binding = 2) uniform PointLightInfo {
    PointLights lights[MAX_POINT_LIGHT_COUNT];
    uint lightCount;
    ivec3 _padding;
} pointLightInfo;

layout (set = 0, binding = 3) uniform sampler2D[5] pbrImages;

void main() {
    color = vec4(1, 0, 0, 1);
}