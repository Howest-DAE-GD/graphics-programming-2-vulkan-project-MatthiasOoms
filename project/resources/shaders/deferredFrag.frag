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
//layout(binding = 2) uniform sampler2D normalSampler;

void main()
{
    //vec3 normalMapSample = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;

    // build TBN matrix from interpolated vectors
    mat3 TBN = mat3(normalize(fragTangent), normalize(fragBitangent), normalize(fragNormal));
    
    //vec3 worldNormal = normalize(TBN * normalMapSample);

    outAlbedo = texture(texSampler, fragTexCoord);
    outNormal = vec4(fragNormal, 1.0);
    outPosition = vec4(fragPosition, 1.0);
}
