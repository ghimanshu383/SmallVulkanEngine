#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 normals;

layout (set = 0, binding = 0) uniform CameraUBO {
    mat4 projection;
    mat4 view;
    vec4 position;
} cameraUBO;

layout (push_constant) uniform Model {
    mat4 model;
    uint pickId;
} model;


layout (location = 0) out vec2 texCoordinates;
layout (location = 1) out vec3 vNormals;
layout (location = 2) out vec3 vCameraPos;
layout (location = 3) out vec3 vWorldPos;
layout (location = 4) out flat uint vPickId;

void main() {
    vec4 worldPos = model.model * vec4(pos, 1.0);
    vec4 clip = cameraUBO.projection * cameraUBO.view * worldPos;
    gl_Position = clip;

    texCoordinates = uv;
    vCameraPos = cameraUBO.position.xyz;
    vNormals = mat3(transpose(inverse(model.model))) * normals;
    vWorldPos = worldPos.xyz;
    vPickId = model.pickId;

}