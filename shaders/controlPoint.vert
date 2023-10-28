#version 440

layout(location = 0) in vec4 position;

layout(push_constant) uniform PC {
    mat4 mvp;
} pc;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = pc.mvp * position;
}