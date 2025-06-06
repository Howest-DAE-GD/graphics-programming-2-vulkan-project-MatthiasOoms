#version 450

layout(push_constant) uniform PushConstants {
    vec4 screenSize;
    vec4 cameraForward;
} pushConstants;

layout(location = 0) out vec4 outColor;

layout(binding = 4) uniform sampler2D gAlbedo;
layout(binding = 5) uniform sampler2D gNormal;
layout(binding = 6) uniform sampler2D gMetalRough;

void main()
{
    vec2 uv = gl_FragCoord.xy / pushConstants.screenSize.xy;

    vec4 albedo = texture(gAlbedo, uv);
    vec4 normalTex = texture(gNormal, uv);
    vec2 metalRough = texture(gMetalRough, uv).bg;

    float metallic = metalRough.r;
    float roughness = clamp(1 - metalRough.g, 0.05, 1.0);

    // Decode normal from [0,1] to [-1,1]
    vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);

    // Simple hard-coded directional light
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));

    // Lambert diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // View direction (assuming camera is at origin looking down -Z)
    vec3 viewDir = normalize(-pushConstants.cameraForward.xyz);

    // Simple fake specular highlight (no BRDF yet)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0 * (1.0 - roughness));

    // Fresnel term approximation
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    vec3 fresnel = F0 + (1.0 - F0) * pow(1.0 - max(dot(viewDir, halfwayDir), 0.0), 5.0);

    vec3 diffuse = albedo.rgb * (1.0 - metallic);
    vec3 specular = fresnel * spec;

    vec3 color = (diff * diffuse + specular) + albedo.rgb * 0.05; // ambient

    outColor = vec4(color, albedo.a);
}