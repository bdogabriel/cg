#version 460 core

layout(std430, binding = 1) buffer FaceColorBuffer {
    uint faceColors[];
};

flat in int faceOffset;
out vec4 fragColor;

void main()
{
    fragColor = unpackUnorm4x8(faceColors[faceOffset + gl_PrimitiveID]);
}
