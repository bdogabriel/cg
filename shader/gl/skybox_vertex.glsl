#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 u_viewProj;

out vec3 vDir;

void main()
{
    vDir = aPos;
    gl_Position = (u_viewProj * vec4(aPos, 1.0)).xyww;
}
