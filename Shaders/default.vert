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
layout (location = 4) out vec3 vWorldPos;
layout (location = 5) out vec3 vPos;
layout (location = 6) flat out uint vPickId;

layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 projection;
    mat4 view;
} vp;

layout (set = 0, binding = 1) uniform ModelUBO {
    mat4 model;
    uint pickId;
} model;

layout (location = 0) out vec4 vColor;
layout (location = 1) out vec2 textureCoords;
layout (location = 2) out vec3 vNormals;

layout (push_constant) uniform ModelPush {
    mat4 model;
} modelPush;

void main() {
    vec4 worldPos = model.model * vec4(pos, 1.0);
    gl_Position = vp.projection * vp.view * worldPos;
    vColor = color;
    textureCoords = uv;
    vNormals = normals;
    vWorldPos = worldPos.xyz;

    vNormals = mat3(transpose(inverse(modelPush.model))) * normals;
    vPos = pos;
    vPickId = model.pickId;
}