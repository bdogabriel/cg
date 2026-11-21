#version 460 core

uniform sampler2D u_skybox;

in vec3 vDir;
out vec4 fragColor;

const float PI = 3.14159265359;

void main()
{
    vec3 d = normalize(vDir);
    float u = atan(d.z, d.x) / (2.0 * PI) + 0.5;
    float v = asin(clamp(d.y, -1.0, 1.0)) / PI + 0.5;
    fragColor = texture(u_skybox, vec2(u, v));
}
