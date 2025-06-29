#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec3 fragBitangent;
layout(location = 4) out vec2 fragTexCoord;

void main()
{
    fragPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    fragTangent = normalize(normalMatrix * inTangent.xyz);
    fragBitangent = normalize(cross(fragNormal, fragTangent) * inTangent.w);
    if (inTangent.w == 0.0) 
    {
        fragBitangent = normalize(cross(fragNormal, fragTangent));
    }
    fragTexCoord = inTexCoord;
    gl_Position = ubo.proj * ubo.view * vec4(fragPosition, 1.0);
}
