#version 440

layout(location = 0) in vec3 vECVertNormal;
layout(location = 1) in vec3 vECVertPos;
layout(location = 2) flat in vec3 vDiffuseAdjust;


layout(std140, binding = 1) uniform buf {
    vec3 ECCameraPosition;
    vec3 ka;
    vec3 kd;
    vec3 ks;
    vec3 slicingDir;
    // Have one light only for now.
    vec3 ECLightPosition;
    vec3 attenuation;
    vec3 color;
    float intensity;
    float specularExp;
    float slicingScaling;
} ubuf;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(vDiffuseAdjust, 1.0);
}