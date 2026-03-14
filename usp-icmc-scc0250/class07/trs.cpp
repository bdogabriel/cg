#include "trs.h"
#include <cmath>

void identity(Mat4 &m)
{
    m.col[0] = _mm_set_ps(0, 0, 0, 1);
    m.col[1] = _mm_set_ps(0, 0, 1, 0);
    m.col[2] = _mm_set_ps(0, 1, 0, 0);
    m.col[3] = _mm_set_ps(1, 0, 0, 0);
}

void translate(Mat4 &m, float tx, float ty, float tz)
{
    Mat4 t;

    t.col[0] = _mm_set_ps(0, 0, 0, 1);
    t.col[1] = _mm_set_ps(0, 0, 1, 0);
    t.col[2] = _mm_set_ps(0, 1, 0, 0);
    t.col[3] = _mm_set_ps(1, tz, ty, tx);

    m *= t;
}

void scale(Mat4 &m, float sx, float sy, float sz)
{
    Mat4 s;

    s.col[0] = _mm_set_ps(0, 0, 0, sx);
    s.col[1] = _mm_set_ps(0, 0, sy, 0);
    s.col[2] = _mm_set_ps(0, sz, 0, 0);
    s.col[3] = _mm_set_ps(1, 0, 0, 0);

    m *= s;
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

void build_trs(Mat4 &m, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz)
{
    identity(m);
    translate(m, tx, ty, tz);
    rotate_x(m, rx);
    rotate_y(m, ry);
    rotate_z(m, rz);
    scale(m, sx, sy, sz);
}

void build_trs_direct(Mat4 &m, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz)
{
    float cosX = std::cos(rx), sinX = std::sin(rx);
    float cosY = std::cos(ry), sinY = std::sin(ry);
    float cosZ = std::cos(rz), sinZ = std::sin(rz);

    // c0: x vector (right)
    float c00 = cosY * cosZ;
    float c01 = cosZ * sinY * sinX + sinZ * cosX;
    float c02 = cosZ * sinY * cosX - sinZ * sinX;

    // c1: y vector (up)
    float c10 = cosY * sinZ;
    float c11 = sinZ * sinY * sinX - cosZ * cosX;
    float c12 = sinZ * sinY * cosX + cosZ * sinX;

    // c2: z vector (forward)
    float c20 = -sinY;
    float c21 = cosY * sinX;
    float c22 = cosY * cosX;

    // _mm_set_ps order: (w, z, y, x)
    m.col[0] = _mm_set_ps(0, c02 * sx, c01 * sx, c00 * sx); // right * sx
    m.col[1] = _mm_set_ps(0, c12 * sy, c11 * sy, c10 * sy); // up * sy
    m.col[2] = _mm_set_ps(0, c22 * sz, c21 * sz, c20 * sz); // forward * sz
    m.col[3] = _mm_set_ps(1, tz, ty, tx);
}

void translate(Mat4 &m, const TRS &trs)
{
    translate(m, trs.tx, trs.ty, trs.tz);
}

void rotate(Mat4 &m, const TRS &trs)
{
    rotate_x(m, trs.rx);
    rotate_y(m, trs.ry);
    rotate_z(m, trs.rz);
}

void scale(Mat4 &m, const TRS &trs)
{
    scale(m, trs.sx, trs.sy, trs.sz);
}

void build_trs(Mat4 &m, const TRS &trs)
{
    build_trs(m, trs.tx, trs.ty, trs.tz, trs.rx, trs.ry, trs.rz, trs.sx, trs.sy, trs.sz);
}

void build_trs_direct(Mat4 &m, const TRS &trs)
{
    build_trs_direct(m, trs.tx, trs.ty, trs.tz, trs.rx, trs.ry, trs.rz, trs.sx, trs.sy, trs.sz);
}
