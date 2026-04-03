#ifndef TRS_H
#define TRS_H

#include "mat4.h"

struct TRS
{
    float tx = 0, ty = 0, tz = 0;
    Mat4 r = mat4::IDENTITY;
    float sx = 1, sy = 1, sz = 1;
};

namespace trs
{
Mat4 compose(const TRS &t);
void translate(TRS &t, float tx, float ty, float tz);
void translate_local(TRS &t, float tx, float ty, float tz);
void rotate_x(TRS &t, float rx);
void rotate_y(TRS &t, float ry);
void rotate_z(TRS &t, float rz);
void rotate_x_local(TRS &t, float rx);
void rotate_y_local(TRS &t, float ry);
void rotate_z_local(TRS &t, float rz);
void scale(TRS &t, float sx, float sy, float sz);
} // namespace trs

#endif // TRS_H
