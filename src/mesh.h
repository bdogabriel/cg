#ifndef MESH_H
#define MESH_H

#include "color.h"
#include "polyhedron.h"

// TODO: review face transform and extrude
// TODO: transform/extrude faces into their own local axis (normal as y)
// TODO: use non-triangular faces for selecting/editing (selecting each triangle individually is a pain)
// TODO: add proper color support with color parameter to the functions

struct Mesh
{
    const Vec4 *vertices = nullptr;
    int vtxCount = 0;
    const unsigned int *indices = nullptr;
    int idxCount = 0;
    const Color *faceColors = nullptr;
    int faceCount = 0;
    int faceVtxCount = 3;
    Mat4 model = mat4::IDENTITY;
    const char *name = nullptr;
};

namespace mesh
{

Mesh make_axis(const char axis);
Mesh make_polyhedron(int faceCount);
Mesh make_pyramid(int sides);
Mesh make_cylinder(int segments);

constexpr float PI = 3.14159265359f;

constexpr int MAX_PYRAMID_SIDES = 32;
constexpr int MAX_CYLINDER_SEGMENTS = 32;

} // namespace mesh

#endif // MESH_H
