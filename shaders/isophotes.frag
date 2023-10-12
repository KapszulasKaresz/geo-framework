#version 440

layout(location = 0) in vec3 vECVertNormal;
layout(location = 1) in vec3 vECVertPos;
layout(location = 2) flat in vec3 vDiffuseAdjust;

layout(binding = 2) uniform sampler2D texSampler;

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
    vec3 unnormL = ubuf.ECLightPosition - vECVertPos;
    float dist = length(unnormL);
    float att = 1.0 / (ubuf.attenuation.x + ubuf.attenuation.y * dist + ubuf.attenuation.z * dist * dist);

    vec3 N = normalize(vECVertNormal);
    vec3 L = normalize(unnormL);
    float NL = max(0.0, dot(N, L));

    vec3 r = reflect(normalize(vECVertNormal), N);
    float m = 2. * sqrt(
    pow( r.x, 2. ) +
    pow( r.y, 2. ) +
    pow( r.z + 1., 2. )
    );

    vec2 vN = r.xy / m;

    vec3 base = texture(texSampler,vN).rgb;
    fragColor = vec4(base, 1.);
}