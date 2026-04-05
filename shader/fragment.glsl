#version 460 core

layout(std430, binding = 1) buffer FaceColorBuffer {
    vec4 faceColors[];
};

flat in int faceOffset;
out vec4 fragColor;

void main()
{
    fragColor = faceColors[faceOffset + gl_PrimitiveID];
}
