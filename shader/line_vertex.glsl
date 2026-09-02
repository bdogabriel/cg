#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

out vec4 vColor;

void main()
{
    vec4 p = position;
    p.z = -p.z * 0.1;
    gl_Position = p;
    vColor = color;
}
