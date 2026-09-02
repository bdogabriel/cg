#ifndef TRS_H
#define TRS_H

#include "quat.h"

struct TRS
{
    float tx = 0, ty = 0, tz = 0;
    Vec4 r = quat::identity();
    float sx = 1, sy = 1, sz = 1;
    float kxy = 0, kxz = 0, kyx = 0, kyz = 0, kzx = 0, kzy = 0;
};

namespace trs
{
Mat4 compose(const TRS &t);

Mat4 translation(float tx, float ty, float tz);
Mat4 rotation_x(float rx);
Mat4 rotation_y(float ry);
Mat4 rotation_z(float rz);
Mat4 scaling(float sx, float sy, float sz);
Mat4 shearing(float kxy, float kxz, float kyx, float kyz, float kzx, float kzy);

void translate(TRS &t, float tx, float ty, float tz);
void translate_local(TRS &t, float tx, float ty, float tz);
void rotate_x(TRS &t, float rx);
void rotate_y(TRS &t, float ry);
void rotate_z(TRS &t, float rz);
void rotate_x_local(TRS &t, float rx);
void rotate_y_local(TRS &t, float ry);
void rotate_z_local(TRS &t, float rz);
void scale(TRS &t, float sx, float sy, float sz);
void shear(TRS &t, float dkxy, float dkxz, float dkyx, float dkyz, float dkzx, float dkzy);
} // namespace trs

#endif // TRS_H
