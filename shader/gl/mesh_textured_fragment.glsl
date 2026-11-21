#version 460 core

uniform sampler2D u_tex;

in vec2 vUV;
out vec4 fragColor;

void main()
{
    fragColor = texture(u_tex, vUV);
}
