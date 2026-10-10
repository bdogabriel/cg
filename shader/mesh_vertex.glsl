#version 460 core

layout(location = 0) in vec4 vertex;

uniform mat4 u_viewProj;

layout(std430, binding = 0) buffer ModelBuffer {
    mat4 transforms[];
};

layout(std430, binding = 2) buffer FaceOffsetBuffer {
    int faceOffsets[];
};

flat out int faceOffset;

void main()
{
    faceOffset = faceOffsets[gl_DrawID];
    gl_Position = u_viewProj * transforms[gl_DrawID] * vertex;
}
