#ifndef TRS_H
#define TRS_H

#include "mat4.h"

struct TRS
{
    float tx = 0, ty = 0, tz = 0, rx = 0, ry = 0, rz = 0, sx = 1, sy = 1, sz = 1;
};

namespace trs
{
void translate(Mat4 &m, float tx, float ty, float tz);
void scale(Mat4 &m, float sx, float sy, float sz);
void rotate_x(Mat4 &m, float rx);
void rotate_y(Mat4 &m, float ry);
void rotate_z(Mat4 &m, float rz);

void translate(Mat4 &m, const TRS &t);
void rotate(Mat4 &m, const TRS &t);
void scale(Mat4 &m, const TRS &t);

TRS combine(const TRS &t1, const TRS &t2);
} // namespace trs

#endif // TRS_H
