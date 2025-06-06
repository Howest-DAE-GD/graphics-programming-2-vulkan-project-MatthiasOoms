#version 450

layout(push_constant) uniform PushConstants {
    vec4 screenSize;
    vec4 cameraForward;
} pushConstants;

layout(location = 0) out vec4 outColor;

layout(binding = 4) uniform sampler2D gAlbedo;
layout(binding = 5) uniform sampler2D gNormal;
layout(binding = 6) uniform sampler2D gMetalRough;

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz normal distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

// Schlick-GGX geometry shadowing function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

// Smith’s geometry function combining both view and light geometry terms
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    // Calculate UV coordinates based on fragment position
    vec2 uv = gl_FragCoord.xy / pushConstants.screenSize.xy;

    // Sample textures
    vec4 albedo = texture(gAlbedo, uv);
    vec4 normalTex = texture(gNormal, uv);
    vec2 metalRough = texture(gMetalRough, uv).bg;

    // Extract metallic and roughness values
    float metallic = metalRough.r;
    float roughness = clamp(1.0 - metalRough.g, 0.05, 1.0);

    // Decode normal from [0,1] to [-1,1]
    vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);

    // View vector
    vec3 view = normalize(-pushConstants.cameraForward.xyz);

    // Hardcoded directional light
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));
    vec3 lightColor = vec3(1.0);
    float lightIntensity = 4.0;

    vec3 halfVec = normalize(view + lightDir);

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    float NDF = DistributionGGX(normal, halfVec, roughness);
    float G = GeometrySmith(normal, view, lightDir, roughness);
    vec3 F = FresnelSchlick(max(dot(halfVec, view), 0.0), F0);

    float denom = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
    vec3 specular = (NDF * G * F) / denom;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = albedo.rgb / PI;

    // Final color
    vec3 color = (kD * diffuse + specular) * lightColor * lightIntensity * NdotL;

    // Small ambient value
    vec3 ambient = albedo.rgb * 0.03;
    color += ambient;

    // PBR output
    outColor = vec4(color, albedo.a);
}