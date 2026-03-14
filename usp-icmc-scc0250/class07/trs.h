#pragma once

#include "mat4.h"

struct TRS
{
    float tx = 0, ty = 0, tz = 0, rx = 0, ry = 0, rz = 0, sx = 1, sy = 1, sz = 1;
};

void identity(Mat4 &m);

void translate(Mat4 &m, float tx, float ty, float tz);
void rotate_x(Mat4 &m, float rx);
void rotate_y(Mat4 &m, float ry);
void rotate_z(Mat4 &m, float rz);
void scale(Mat4 &m, float sx, float sy, float sz);
void build_trs(Mat4 &m, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz);
void build_trs_direct(Mat4 &m, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy,
                      float sz);

void translate(Mat4 &m, const TRS &trs);
void rotate(Mat4 &m, const TRS &trs);
void scale(Mat4 &m, const TRS &trs);
void build_trs(Mat4 &m, const TRS &trs);
void build_trs_direct(Mat4 &m, const TRS &trs);
