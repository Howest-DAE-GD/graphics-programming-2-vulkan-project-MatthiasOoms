#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    outAlbedo = vec4(fragColor, 1.0);
    outNormal = vec4(normalize(fragNormal), 1.0);
    outPosition = vec4(fragPosition, 1.0);
}
