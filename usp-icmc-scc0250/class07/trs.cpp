#include "trs.h"
#include <cmath>

void identity(Mat4 &M)
{
    M.col[0] = _mm_set_ps(0, 0, 0, 1);
    M.col[1] = _mm_set_ps(0, 0, 1, 0);
    M.col[2] = _mm_set_ps(0, 1, 0, 0);
    M.col[3] = _mm_set_ps(1, 0, 0, 0);
}

void translate(Mat4 &M, float x, float y, float z)
{
    Mat4 T;

    T.col[0] = _mm_set_ps(0, 0, 0, 1);
    T.col[1] = _mm_set_ps(0, 0, 1, 0);
    T.col[2] = _mm_set_ps(0, 1, 0, 0);
    T.col[3] = _mm_set_ps(1, z, y, x);

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

void rotateX(Mat4 &M, float angle)
{
    float s = std::sin(angle);
    float c = std::cos(angle);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, 0, 0, 1);
    R.col[1] = _mm_set_ps(0, s, c, 0);
    R.col[2] = _mm_set_ps(0, c, -s, 0);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void rotateY(Mat4 &M, float angle)
{
    float c = std::cos(angle);
    float s = std::sin(angle);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, -s, 0, c);
    R.col[1] = _mm_set_ps(0, 0, 1, 0);
    R.col[2] = _mm_set_ps(0, c, 0, s);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void rotateZ(Mat4 &M, float angle)
{
    float c = std::cos(angle);
    float s = std::sin(angle);

    Mat4 R;

    R.col[0] = _mm_set_ps(0, 0, s, c);
    R.col[1] = _mm_set_ps(0, 0, c, -s);
    R.col[2] = _mm_set_ps(0, 1, 0, 0);
    R.col[3] = _mm_set_ps(1, 0, 0, 0);

    M *= R;
}

void buildTRS(float tX, float tY, float tZ, float angX, float angY, float angZ, float sX, float sY, float sZ, Mat4 &M)
{
    float cosX = std::cos(angX), sinX = std::sin(angX);
    float cosY = std::cos(angY), sinY = std::sin(angY);
    float cosZ = std::cos(angZ), sinZ = std::sin(angZ);

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
    M.col[0] = _mm_set_ps(0, c02 * sZ, c01 * sY, c00 * sX); // right
    M.col[1] = _mm_set_ps(0, c12 * sZ, c11 * sY, c10 * sX); // up
    M.col[2] = _mm_set_ps(0, c22 * sZ, c21 * sY, c20 * sX); // forward
    M.col[3] = _mm_set_ps(1, tZ, tY, tX);
}
