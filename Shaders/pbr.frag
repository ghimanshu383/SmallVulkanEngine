#version 450

layout (location = 0) out vec4 color;
layout (location = 1) out uint pickId;

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

layout (location = 0) in vec2 texCoordinates;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec3 vCameraPos;
layout (location = 3) in vec3 vWorldPos;
layout (location = 4) in flat uint vPickId;


// Normal Distribution Function (GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.141592 * denom * denom;
    return a2 / denom;
}

// Geometry Function (Smith’s method)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // from UE4
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel (Schlick Approximation)
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = pow(texture(pbrImages[0], texCoordinates).rgb, vec3(2.2));
    float metallic = texture(pbrImages[2], texCoordinates).r;
    float roughness = texture(pbrImages[3], texCoordinates).r;
    float ao = texture(pbrImages[4], texCoordinates).r;
    // Lighting vectors
    vec3 N = normalize(vNormals);
    vec3 V = normalize(vCameraPos - vWorldPos);
    vec3 L = normalize(lightInfo.position.xyz - vWorldPos);
    vec3 H = normalize(V + L);

    // Radiance
    vec3 radiance = lightInfo.color.rgb * lightInfo.intensities.r;

    // Cook–Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    // kS is Fresnel reflectance
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // metals don’t have diffuse reflection

    // Lambertian diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * albedo / 3.141592;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // Add ambient term (AO)
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 colorOut = ambient + Lo;

    // Gamma correction
    colorOut = pow(colorOut, vec3(1.0 / 2.2));
    color = vec4(colorOut, 1.0);
    pickId = vPickId;
}