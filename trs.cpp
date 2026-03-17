#include "trs.h"
#include <cmath>

void trs::translate(Mat4 &m, float tx, float ty, float tz)
{
    Mat4 t;

    t.col[0] = _mm_set_ps(0, 0, 0, 1);
    t.col[1] = _mm_set_ps(0, 0, 1, 0);
    t.col[2] = _mm_set_ps(0, 1, 0, 0);
    t.col[3] = _mm_set_ps(1, tz, ty, tx);

    m *= t;
}

void trs::scale(Mat4 &m, float sx, float sy, float sz)
{
    Mat4 s;

    s.col[0] = _mm_set_ps(0, 0, 0, sx);
    s.col[1] = _mm_set_ps(0, 0, sy, 0);
    s.col[2] = _mm_set_ps(0, sz, 0, 0);
    s.col[3] = _mm_set_ps(1, 0, 0, 0);

    m *= s;
}

void trs::rotate_x(Mat4 &m, float rx)
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

void trs::rotate_y(Mat4 &m, float ry)
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

void trs::rotate_z(Mat4 &m, float rz)
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

void trs::translate(Mat4 &m, const TRS &t)
{
    translate(m, t.tx, t.ty, t.tz);
}

void trs::rotate(Mat4 &m, const TRS &t)
{
    rotate_x(m, t.rx);
    rotate_y(m, t.ry);
    rotate_z(m, t.rz);
}

void trs::scale(Mat4 &m, const TRS &t)
{
    scale(m, t.sx, t.sy, t.sz);
}
