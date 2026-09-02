#ifndef MESH_H
#define MESH_H

#include "color.h"
#include "mat4.h"

// TODO: transform/extrude faces into their own local axis (normal as y)
// TODO: use non-triangular faces for selecting/editing (selecting each triangle individually is a pain)
// TODO: add proper color support with color parameter to the functions
// TODO: add make_sphere with u-v sphere
// TODO: add parent object

struct Mesh
{
    const Vec4 *vertices = nullptr;
    int vtxCount = 0;
    const unsigned int *faceCorners = nullptr;
    const int *faceCornerCounts = nullptr;
    int faceCount = 0;
    const Color *faceColors = nullptr;
    Mat4 model = mat4::IDENTITY;
    const char *name = nullptr;
};

namespace mesh
{

Mesh make_polyhedron(int faceCount);
Mesh make_pyramid(int sides);
Mesh make_cylinder(int segments);

constexpr float PI = 3.14159265359f;

constexpr int MAX_PYRAMID_SIDES = 32;
constexpr int MAX_CYLINDER_SEGMENTS = 32;

constexpr int MAX_VERTICES = 8000;
constexpr int MAX_INDICES = 24000;
constexpr int MAX_REFS = 2000;

} // namespace mesh

#endif // MESH_H
