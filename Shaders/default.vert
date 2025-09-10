#version 450

vec2 vert[3] = {
vec2(0, -.5),
vec2(-0.5, 0),
vec2(0.5, 0)
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 normals;

layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} vp;

layout (location = 0) out vec4 vColor;
layout (location = 1) out vec2 textureCoords;
layout (location = 2) out vec3 vNormals;
void main() {
    gl_Position = vp.projection * vp.view * mat4(1) * vec4(pos, 1.f);
    vColor = color;
    textureCoords = uv;
    vNormals = normals;
}