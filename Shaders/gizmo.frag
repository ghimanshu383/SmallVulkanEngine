#version 450

layout (location = 0) in vec4 vColor;
layout (location = 1) in flat uint vId;
layout (location = 2) in flat uint vPickId;

layout (location = 0) out vec4 color;
layout (location = 1) out uint id;

void main() {
    uint axisId = vId % 1000;
    vec3 baseColor = vColor.rgb;
    if (axisId == vPickId) {
        baseColor = mix(baseColor, vec3(1.0), 0.3);
    }
    color = vec4(baseColor, 1.0);
    id = vId;
}