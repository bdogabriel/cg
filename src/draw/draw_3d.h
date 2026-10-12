#ifndef DRAW_3D_H
#define DRAW_3D_H

#include "mesh.h"

namespace draw3d
{
Mesh polyhedron(int faceCount);
Mesh pyramid(int sides);
Mesh cylinder(int segments);
Mesh sphere(int segments);
} // namespace draw3d

#endif
