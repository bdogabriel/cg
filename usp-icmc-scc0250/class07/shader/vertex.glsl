#version 430 core

layout(location = 0) in vec4 v;
layout(location = 0) uniform mat4 trs;

void main() {
    gl_Position = trs * v;
}
