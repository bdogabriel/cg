#pragma once

#include "mat4.h"

void identity(Mat4 &M);
void translate(Mat4 &M, float x, float y, float z);
void scale(Mat4 &M, float sx, float sy, float sz);
void rotateX(Mat4 &M, float angle);
void rotateY(Mat4 &M, float angle);
void rotateZ(Mat4 &M, float angle);
void buildTRS(float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz, Mat4 &M);
