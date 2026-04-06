#version 460 core

layout(location = 0) in vec4 vertex;

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
    vec4 pos = transforms[gl_DrawID] * vertex;
    pos.z = -pos.z * 0.1; // flip z axis because there is no view matrix (already in clip space); scale down to expand near/far range
    gl_Position = pos;
}
