#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragBitangent;
layout(location = 4) in vec2 fragTexCoord;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D normalSampler;

void main()
{
    vec3 normalMap = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;

    vec3 T = normalize(fragTangent);
    vec3 N = normalize(fragNormal);
    vec3 B = normalize(fragBitangent);

    mat3 TBN = mat3(T, B, N);
    vec3 worldNormal = normalize(TBN * normalMap);

    outNormal = vec4(worldNormal * 0.5 + 0.5, 1.0);
    outAlbedo = texture(texSampler, fragTexCoord);
    outPosition = vec4(fragPosition, 1.0);
}
