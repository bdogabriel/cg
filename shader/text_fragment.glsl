#version 460 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D atlas;

out vec4 fragColor;

void main()
{
    float alpha = texture(atlas, vUV).r;
    fragColor = vec4(vColor.rgb, vColor.a * alpha);
}
