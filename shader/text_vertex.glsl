#version 460 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 vertex_uv;
layout(location = 2) in vec4 vertex_color;

uniform vec2 fbSize;

out vec2 vUV;
out vec4 vColor;

void main()
{
    vec2 clip = (vertex_pos / fbSize) * 2.0 - vec2(1.0, 1.0);
    gl_Position = vec4(clip.x, -clip.y, 0.0, 1.0);
    vUV = vertex_uv;
    vColor = vertex_color;
}
