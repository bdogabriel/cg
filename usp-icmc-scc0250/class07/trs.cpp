#include "trs.h"
#include <cmath>

void identity(Mat4 &M)
{
    M.col[0] = _mm_set_ps(0, 0, 0, 1);
    M.col[1] = _mm_set_ps(0, 0, 1, 0);
    M.col[2] = _mm_set_ps(0, 1, 0, 0);
    M.col[3] = _mm_set_ps(1, 0, 0, 0);
}

void translate(Mat4 &M, float tx, float ty, float tz)
{
    Mat4 T;

    T.col[0] = _mm_set_ps(0, 0, 0, 1);
    T.col[1] = _mm_set_ps(0, 0, 1, 0);
    T.col[2] = _mm_set_ps(0, 1, 0, 0);
    T.col[3] = _mm_set_ps(1, tz, ty, tx);

    M *= T;
}

void scale(Mat4 &M, float sx, float sy, float sz)
{
    Mat4 S;

    S.col[0] = _mm_set_ps(0, 0, 0, sx);
    S.col[1] = _mm_set_ps(0, 0, sy, 0);
    S.col[2] = _mm_set_ps(0, sz, 0, 0);
    S.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= S;
}

void rotate_x(Mat4 &M, float rx)
{
    float s = std::sin(rx);
    float c = std::cos(rx);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, 0, 0, 1);
    R.col[1] = _mm_set_ps(0, s, c, 0);
    R.col[2] = _mm_set_ps(0, c, -s, 0);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void rotate_y(Mat4 &M, float ry)
{
    float c = std::cos(ry);
    float s = std::sin(ry);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, -s, 0, c);
    R.col[1] = _mm_set_ps(0, 0, 1, 0);
    R.col[2] = _mm_set_ps(0, c, 0, s);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void rotate_z(Mat4 &M, float rz)
{
    float c = std::cos(rz);
    float s = std::sin(rz);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, 0, s, c);
    R.col[1] = _mm_set_ps(0, 0, c, -s);
    R.col[2] = _mm_set_ps(0, 1, 0, 0);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void build_trs(Mat4 &M, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz)
{
    float cosX = std::cos(rx), sinX = std::sin(rx);
    float cosY = std::cos(ry), sinY = std::sin(ry);
    float cosZ = std::cos(rz), sinZ = std::sin(rz);

    // c0: x vector (right)
    float c00 = cosY * cosZ;
    float c01 = cosZ * sinY * sinX - sinZ * cosX;
    float c02 = cosZ * sinY * cosX + sinZ * sinX;

    // c1: y vector (up)
    float c10 = cosY * sinZ;
    float c11 = sinZ * sinY * sinX + cosZ * cosX;
    float c12 = sinZ * sinY * cosX - cosZ * sinX;

    // c2: z vector (forward)
    float c20 = -sinY;
    float c21 = cosY * sinX;
    float c22 = cosY * cosX;

    // _mm_set_ps order: (w, z, y, x)
    M.col[0] = _mm_set_ps(0, c02 * sz, c01 * sy, c00 * sx); // right
    M.col[1] = _mm_set_ps(0, c12 * sz, c11 * sy, c10 * sx); // up
    M.col[2] = _mm_set_ps(0, c22 * sz, c21 * sy, c20 * sx); // forward
    M.col[3] = _mm_set_ps(1, tz, ty, tx);
}

void translate(Mat4 &M, const TRS &trs)
{
    translate(M, trs.tx, trs.ty, trs.tz);
}

void rotate(Mat4 &M, const TRS &trs)
{
    rotate_x(M, trs.rx);
    rotate_y(M, trs.ry);
    rotate_z(M, trs.rz);
}

void scale(Mat4 &M, const TRS &trs)
{
    scale(M, trs.sx, trs.sy, trs.sz);
}

void build_trs(Mat4 &M, const TRS &trs)
{
    build_trs(M, trs.tx, trs.ty, trs.tz, trs.rx, trs.ry, trs.rz, trs.sx, trs.sy, trs.sz);
}
