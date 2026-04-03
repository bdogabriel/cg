#version 460 core

layout(location = 0) in vec4 vertex;

layout(std430, binding = 0) buffer TransformBuffer {
    mat4 transforms[];
};

layout(std430, binding = 1) buffer ColorBuffer {
    vec4 colors[];
};

out vec4 vColor;

void main()
{
    mat4 transform = transforms[gl_DrawID];
    vec4 color = colors[gl_DrawID];
    vColor = color;
    gl_Position = transform * vertex;
}
