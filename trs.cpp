#include "trs.h"
#include "mat4.h"
#include <cmath>

// NOTE: rotation accumulates float error -> axes lose orthogonality and unit length
// TODO: re-orthogonalize periodically via Gram-Schmidt or use quaternions

namespace trs
{
Mat4 compose(const TRS &t)
{
    Mat4 m = t.r;
    m.col[0] = _mm_mul_ps(m.col[0], _mm_set1_ps(t.sx));
    m.col[1] = _mm_mul_ps(m.col[1], _mm_set1_ps(t.sy));
    m.col[2] = _mm_mul_ps(m.col[2], _mm_set1_ps(t.sz));
    m.col[3] = _mm_set_ps(1, t.tz, t.ty, t.tx);

    __m128 c0 = m.col[0], c1 = m.col[1], c2 = m.col[2];
    m.col[0] = _mm_add_ps(c0, _mm_add_ps(_mm_mul_ps(c1, _mm_set1_ps(t.kyx)), _mm_mul_ps(c2, _mm_set1_ps(t.kzx))));
    m.col[1] = _mm_add_ps(c1, _mm_add_ps(_mm_mul_ps(c0, _mm_set1_ps(t.kxy)), _mm_mul_ps(c2, _mm_set1_ps(t.kzy))));
    m.col[2] = _mm_add_ps(c2, _mm_add_ps(_mm_mul_ps(c0, _mm_set1_ps(t.kxz)), _mm_mul_ps(c1, _mm_set1_ps(t.kyz))));
    return m;
}

Mat4 translation(float tx, float ty, float tz)
{
    Mat4 m;
    m.col[0] = _mm_set_ps(0, 0, 0, 1);
    m.col[1] = _mm_set_ps(0, 0, 1, 0);
    m.col[2] = _mm_set_ps(0, 1, 0, 0);
    m.col[3] = _mm_set_ps(1, tz, ty, tx);
    return m;
}

Mat4 rotation_x(float rx)
{
    float cos = std::cos(rx);
    float sin = std::sin(rx);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, 0, 0, 1);
    r.col[1] = _mm_set_ps(0, sin, cos, 0);
    r.col[2] = _mm_set_ps(0, cos, -sin, 0);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    return r;
}

Mat4 rotation_y(float ry)
{
    float cos = std::cos(ry);
    float sin = std::sin(ry);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, -sin, 0, cos);
    r.col[1] = _mm_set_ps(0, 0, 1, 0);
    r.col[2] = _mm_set_ps(0, cos, 0, sin);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    return r;
}

Mat4 rotation_z(float rz)
{
    float cos = std::cos(rz);
    float sin = std::sin(rz);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, 0, sin, cos);
    r.col[1] = _mm_set_ps(0, 0, cos, -sin);
    r.col[2] = _mm_set_ps(0, 1, 0, 0);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    return r;
}

Mat4 scaling(float sx, float sy, float sz)
{
    Mat4 m;
    m.col[0] = _mm_set_ps(0, 0, 0, sx);
    m.col[1] = _mm_set_ps(0, 0, sy, 0);
    m.col[2] = _mm_set_ps(0, sz, 0, 0);
    m.col[3] = _mm_set_ps(1, 0, 0, 0);
    return m;
}

// kpq shears p along q axis
Mat4 shearing(float kxy, float kxz, float kyx, float kyz, float kzx, float kzy)
{
    Mat4 m;
    m.col[0] = _mm_set_ps(0, kzx, kyx, 1);
    m.col[1] = _mm_set_ps(0, kzy, 1, kxy);
    m.col[2] = _mm_set_ps(0, 1, kyz, kxz);
    m.col[3] = _mm_set_ps(1, 0, 0, 0);
    return m;
}

void translate(TRS &t, float tx, float ty, float tz)
{
    t.tx += tx;
    t.ty += ty;
    t.tz += tz;
}

void translate_local(TRS &t, float tx, float ty, float tz)
{
    const float *d = t.r.data();
    t.tx += d[0] * tx + d[4] * ty + d[8] * tz;
    t.ty += d[1] * tx + d[5] * ty + d[9] * tz;
    t.tz += d[2] * tx + d[6] * ty + d[10] * tz;
}

void rotate_x(TRS &t, float rx)
{
    t.r = rotation_x(rx) * t.r;
}
void rotate_y(TRS &t, float ry)
{
    t.r = rotation_y(ry) * t.r;
}
void rotate_z(TRS &t, float rz)
{
    t.r = rotation_z(rz) * t.r;
}

void rotate_x_local(TRS &t, float rx)
{
    t.r = t.r * rotation_x(rx);
}
void rotate_y_local(TRS &t, float ry)
{
    t.r = t.r * rotation_y(ry);
}
void rotate_z_local(TRS &t, float rz)
{
    t.r = t.r * rotation_z(rz);
}

void scale(TRS &t, float sx, float sy, float sz)
{
    t.sx *= sx;
    t.sy *= sy;
    t.sz *= sz;
}

void shear(TRS &t, float dkxy, float dkxz, float dkyx, float dkyz, float dkzx, float dkzy)
{
    t.kxy += dkxy;
    t.kxz += dkxz;
    t.kyx += dkyx;
    t.kyz += dkyz;
    t.kzx += dkzx;
    t.kzy += dkzy;
}
} // namespace trs
