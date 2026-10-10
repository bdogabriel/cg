#include "view_projection.h"

namespace viewproj
{

// right-handed perspective projection
// maps view-space -z to clip-space [-1, 1] (near -> -1, far -> +1).
Mat4 perspective(float fovY, float aspect, float near, float far)
{
    float f = 1.0f / tanf(fovY * 0.5f);
    float depth = (near + far) / (near - far);
    float depth2 = (2.0f * near * far) / (near - far);

    Mat4 m;
    m.col[0] = _mm_set_ps(0, 0, 0, f / aspect);
    m.col[1] = _mm_set_ps(0, 0, f, 0);
    m.col[2] = _mm_set_ps(-1, depth, 0, 0);
    m.col[3] = _mm_set_ps(0, depth2, 0, 0);
    return m;
}

// right-handed look-at view
Mat4 look_at(const Vec4 &eye, const Vec4 &center, const Vec4 &up)
{
    Vec4 f = vec4::normalize(center - eye);
    Vec4 s = vec4::normalize(vec4::cross(f, up));
    Vec4 u = vec4::cross(s, f);

    Mat4 m;
    m.col[0] = _mm_set_ps(0, -f.x, u.x, s.x);
    m.col[1] = _mm_set_ps(0, -f.y, u.y, s.y);
    m.col[2] = _mm_set_ps(0, -f.z, u.z, s.z);
    m.col[3] = _mm_set_ps(1, vec4::dot3(f, eye), -vec4::dot3(u, eye), -vec4::dot3(s, eye));
    return m;
}

} // namespace viewproj
