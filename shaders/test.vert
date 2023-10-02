#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

// Instanced attributes to variate the translation of the model and the diffuse
// color of the material.
layout(location = 2) in vec3 instTranslate;
layout(location = 3) in vec3 instDiffuseAdjust;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 vECVertNormal;
layout(location = 1) out vec3 vECVertPos;
layout(location = 2) flat out vec3 vDiffuseAdjust;

layout(std140, binding = 0) uniform buf {
    mat4 vp;
    mat4 model;
    mat3 modelNormal;
} ubuf;

void main()
{
    vECVertNormal = vECVertNormal;
    vECVertPos = vec3(1,1,1);
    vDiffuseAdjust = instDiffuseAdjust;
    gl_Position = vec4(1,1,1,1);
}