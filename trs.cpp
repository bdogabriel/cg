#include "trs.h"
#include <cmath>

namespace trs
{
void translate(Mat4 &m, float tx, float ty, float tz)
{
    m.col[3] = _mm_add_ps(m.col[3], _mm_set_ps(0, tz, ty, tx));
}

void scale(Mat4 &m, float sx, float sy, float sz)
{
    m.col[0] = _mm_mul_ps(m.col[0], _mm_set1_ps(sx));
    m.col[1] = _mm_mul_ps(m.col[1], _mm_set1_ps(sy));
    m.col[2] = _mm_mul_ps(m.col[2], _mm_set1_ps(sz));
}

void rotate_x(Mat4 &m, float rx)
{
    float cos = std::cos(rx);
    float sin = std::sin(rx);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, 0, 0, 1);
    r.col[1] = _mm_set_ps(0, sin, cos, 0);
    r.col[2] = _mm_set_ps(0, cos, -sin, 0);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    m *= r;
}

void rotate_y(Mat4 &m, float ry)
{
    float cos = std::cos(ry);
    float sin = std::sin(ry);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, -sin, 0, cos);
    r.col[1] = _mm_set_ps(0, 0, 1, 0);
    r.col[2] = _mm_set_ps(0, cos, 0, sin);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    m *= r;
}

void rotate_z(Mat4 &m, float rz)
{
    float cos = std::cos(rz);
    float sin = std::sin(rz);

    Mat4 r;

    r.col[0] = _mm_set_ps(0, 0, sin, cos);
    r.col[1] = _mm_set_ps(0, 0, cos, -sin);
    r.col[2] = _mm_set_ps(0, 1, 0, 0);
    r.col[3] = _mm_set_ps(1, 0, 0, 0);

    m *= r;
}

void translate(Mat4 &m, const TRS &t)
{
    translate(m, t.tx, t.ty, t.tz);
}

void rotate(Mat4 &m, const TRS &t)
{
    rotate_x(m, t.rx);
    rotate_y(m, t.ry);
    rotate_z(m, t.rz);
}

void scale(Mat4 &m, const TRS &t)
{
    scale(m, t.sx, t.sy, t.sz);
}

TRS combine(const TRS &t1, const TRS &t2)
{
    TRS r;

    r.tx = t1.tx + t2.tx;
    r.ty = t1.ty + t2.ty;
    r.tz = t1.tz + t2.tz;

    r.rx = t1.rx + t2.rx;
    r.ry = t1.ry + t2.ry;
    r.rz = t1.rz + t2.rz;

    r.sx = t1.sx * t2.sx;
    r.sy = t1.sy * t2.sy;
    r.sz = t1.sz * t2.sz;

    return r;
}
} // namespace trs
