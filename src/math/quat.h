#ifndef QUAT_H
#define QUAT_H

#include "mat4.h"

namespace quat
{
inline Vec4 identity() noexcept
{
    return Vec4{0, 0, 0, 1};
}

inline Vec4 from_euler_x(float rx) noexcept
{
    float h = rx * 0.5f;
    return Vec4{sinf(h), 0, 0, cosf(h)};
}

inline Vec4 from_euler_y(float ry) noexcept
{
    float h = ry * 0.5f;
    return Vec4{0, sinf(h), 0, cosf(h)};
}

inline Vec4 from_euler_z(float rz) noexcept
{
    float h = rz * 0.5f;
    return Vec4{0, 0, sinf(h), cosf(h)};
}

inline Vec4 mul(const Vec4 &a, const Vec4 &b) noexcept
{
    return Vec4{a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
                a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
}

inline Vec4 normalize(const Vec4 &q) noexcept
{
    float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len < 1e-8f)
    {
        return identity();
    }
    float inv = 1.0f / len;
    return Vec4{q.x * inv, q.y * inv, q.z * inv, q.w * inv};
}

inline Vec4 conjugate(const Vec4 &q) noexcept
{
    return Vec4{-q.x, -q.y, -q.z, q.w};
}

inline Vec4 rotate(const Vec4 &q, const Vec4 &v) noexcept
{
    Vec4 qv = Vec4{q.x, q.y, q.z, 0};
    Vec4 t = vec4::cross(qv, v) * 2.0f;
    Vec4 out = v + (t * q.w) + vec4::cross(qv, t);
    out.w = v.w;
    return out;
}

inline Mat4 to_mat4(const Vec4 &q) noexcept
{
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    Mat4 m;
    m.col[0] = _mm_set_ps(0, 2 * (xz - wy), 2 * (xy + wz), 1 - 2 * (yy + zz));
    m.col[1] = _mm_set_ps(0, 2 * (yz + wx), 1 - 2 * (xx + zz), 2 * (xy - wz));
    m.col[2] = _mm_set_ps(0, 1 - 2 * (xx + yy), 2 * (yz - wx), 2 * (xz + wy));
    m.col[3] = _mm_set_ps(1, 0, 0, 0);
    return m;
}
} // namespace quat

#endif
