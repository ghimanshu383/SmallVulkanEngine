#version 450

layout (location = 0) in vec3 vPos;
layout (location = 0 ) out vec4 color;
void main() {
    float depth = gl_FragCoord.z;
    color = vec4(depth, depth, depth, 1.0);
}