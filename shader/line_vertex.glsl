#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 u_viewProj;

out vec4 vColor;

void main()
{
    gl_Position = u_viewProj * position;
    vColor = color;
}
