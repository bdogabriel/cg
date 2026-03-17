#pragma once

#include "geometry.h"

struct Entity
{
    GeometryBuffer geometry;
    TRS transform;
    Mat4 matrix;
    Mat4 rotation = Mat4::identity(); // must accumulate
    Color color;
    GLenum primitive = GL_TRIANGLES;

    void build_matrix();
    void draw(GLint locTransform, GLint locColor) const;
};
