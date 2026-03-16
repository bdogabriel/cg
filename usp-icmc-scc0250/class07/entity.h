#pragma once

#include "geometry.h"

struct Entity
{
    GeometryBuffer geometry;
    TRS transform;
    Color color;
    Mat4 matrix;

    void build();
    void draw(GLint locT, GLint locC) const;
};
