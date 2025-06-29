#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    vec4 texColor = texture(texSampler, fragTexCoord);

    if (texColor.a < 0.5)
        discard;

    outColor = texColor;
}
