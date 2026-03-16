#version 430 core

layout(location = 0) in vec4 vertex;
layout(location = 0) uniform mat4 transform;

void main() {
    gl_Position = transform * vertex;
}
