#version 450

layout (location = 0) in vec4 vColor;
layout (location = 1) in vec2 textureCoords;
layout (location = 2) in vec3 vNormals;
layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D defaultSampler;



layout (set = 2, binding = 0) uniform OmniDirectionalInfo {
    vec3 position;
    vec4 color;
    float ambientIntensity;
    float diffuseIntensity;
} lightInfo;

vec4 CalculatePongLights() {
    vec4 ambientLight = lightInfo.ambientIntensity * lightInfo.color;
    float diffuseFactor = max(dot(normalize(vNormals), normalize(lightInfo.position)), 0.0);
    vec4 diffuseLight = lightInfo.color * diffuseFactor * lightInfo.diffuseIntensity;

    return (ambientLight + diffuseLight);
}
void main() {
    color = texture(defaultSampler, textureCoords) * CalculatePongLights();
}