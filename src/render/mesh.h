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
    const Vec2 *uvs = nullptr;
    const int *faceMaterialIds = nullptr;
    Mat4 model = mat4::IDENTITY;
    const char *name = nullptr;
};

namespace mesh
{

constexpr float PI = 3.14159265359f;

} // namespace mesh

#endif // MESH_H
