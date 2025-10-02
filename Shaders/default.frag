#version 450

layout (location = 0) in vec4 vColor;
layout (location = 1) in vec2 textureCoords;
layout (location = 2) in vec3 vNormals;
layout (location = 4) in vec3 vWorldPos;
layout (location = 5) in vec3 vPos;
layout (location = 6) flat in uint vPickId;

layout (location = 0) out vec4 color;
layout (location = 1) out uint id;

layout (set = 1, binding = 0) uniform sampler2D defaultSampler;

layout (set = 2, binding = 0) uniform OmniDirectionalInfo {
    mat4 projection;
    mat4 view;
    vec4 position;
    vec4 color;
    vec4 intensities;
} lightInfo;

layout (set = 3, binding = 0) uniform sampler2D shadowSampler;

struct PointLights {
    vec4 position;
    vec4 color;
    vec4 intensities;
};
const int MAX_POINT_LIGHT_COUNT = 10;
layout (set = 4, binding = 0) uniform PointLightInfo {
    PointLights lights[MAX_POINT_LIGHT_COUNT];
    int lightCount;
    ivec3 _padding;
} pointLightInfo;

float CalShadowFactor() {
    vec4 lightSpace = lightInfo.projection * lightInfo.view * vec4(vWorldPos, 1.0);
    vec3 proj = lightSpace.xyz / lightSpace.w;
    vec3 normalizedProj = proj * 0.5f + 0.5f;
    float bias = .005;
    float shadowDepth = texture(shadowSampler, normalizedProj.xy).r;
    float current = clamp(proj.z, 0.0, 1.0);
    float diff = current - shadowDepth;

    return current - bias > shadowDepth ? 0 : 1;
}
vec4 CalculatePointLights() {
    return pointLightInfo.lights[0].color;
}
vec4 CalculatePongLights() {
    vec4 ambientLight = lightInfo.intensities.x * lightInfo.color;

    vec3 lightDir = normalize(lightInfo.position.xyz - vWorldPos);
    float diffuseFactor = max(dot(normalize(vNormals), lightDir), 0.0);
    vec4 diffuseLight = lightInfo.color * lightInfo.intensities.y * diffuseFactor * CalShadowFactor();

    return (ambientLight + diffuseLight);
}
void main() {
    color = texture(defaultSampler, textureCoords) * CalculatePongLights() * CalculatePointLights();
    id = vPickId;
}