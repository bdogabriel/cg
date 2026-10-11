#version 460 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 uv;

uniform mat4 u_viewProj;

layout(std430, binding = 0) buffer ModelBuffer {
    mat4 transforms[];
};

out vec2 vUV;

void main()
{
    vUV = uv;
    gl_Position = u_viewProj * transforms[gl_BaseInstance] * vertex;
}
