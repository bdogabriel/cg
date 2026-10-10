#ifndef DRAW_3D_H
#define DRAW_3D_H

#include "mesh.h"

namespace draw3d
{
constexpr int MAX_PYRAMID_SIDES = 32;
constexpr int MAX_CYLINDER_SEGMENTS = 32;

Mesh polyhedron(int faceCount);
Mesh pyramid(int sides);
Mesh cylinder(int segments);
} // namespace draw3d

#endif
