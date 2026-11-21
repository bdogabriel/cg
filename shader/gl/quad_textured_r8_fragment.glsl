#version 460 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D tex;

out vec4 fragColor;

void main()
{
    float alpha = texture(tex, vUV).r;
    fragColor = vec4(vColor.rgb, vColor.a * alpha);
}
