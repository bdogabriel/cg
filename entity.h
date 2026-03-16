#pragma once

#include "geometry.h"

struct Entity
{
    GeometryBuffer geometry;
    TRS transform;
    Color color;
    Mat4 matrix;
    GLenum primitive = GL_TRIANGLES;
    Mat4 *parentMatrix = nullptr;

    void build();
    void draw(GLint locTransform, GLint locColor) const;
};
