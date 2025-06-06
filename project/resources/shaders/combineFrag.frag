#version 450

layout(push_constant) uniform PushConstants {
    vec2 screenSize;
} pushConstants;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 4) uniform sampler2D gAlbedo;
layout(binding = 5) uniform sampler2D gNormal;
layout(binding = 6) uniform sampler2D gMetalRough;

void main()
{
    vec2 uv = gl_FragCoord.xy / pushConstants.screenSize;
    vec4 albedo = texture(gAlbedo, uv);
    vec4 normalTex = texture(gNormal, uv);

    // Decode normal from [0,1] to [-1,1]
    vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);

    // Simple lighting direction for demo (e.g., directional light from above-front)
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));

    // Lambertian diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);

    // Modulate albedo by diffuse lighting
    vec3 litColor = albedo.rgb * diff;

    // Optional: add some ambient term to avoid pure black shadows
    vec3 ambient = albedo.rgb * 0.1;

    vec3 finalColor = litColor + ambient;

    outColor = vec4(finalColor, albedo.a);
}