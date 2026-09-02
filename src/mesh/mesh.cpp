#include "mesh.h"
#include "polyhedron.h"

#include <stdio.h>

namespace mesh
{

static Vec4 pyramidVertices[MAX_PYRAMID_SIDES + 2];
static unsigned int pyramidFaceCorners[MAX_PYRAMID_SIDES * 6];
static int pyramidFaceCornerCounts[MAX_PYRAMID_SIDES * 2];
static Color pyramidColors[MAX_PYRAMID_SIDES * 2];

static Vec4 cylinderVertices[MAX_CYLINDER_SEGMENTS * 2 + 2];
static unsigned int cylinderFaceCorners[MAX_CYLINDER_SEGMENTS * 12];
static int cylinderFaceCornerCounts[MAX_CYLINDER_SEGMENTS * 4];
static Color cylinderColors[MAX_CYLINDER_SEGMENTS * 4];

static Color defaultColors[20];
static bool defaultColorsInit = false;

static void ensureDefaultColors()
{
    if (!defaultColorsInit)
    {
        for (int i = 0; i < 20; i++)
        {
            defaultColors[i] = {255, 255, 255, 255};
        }
        defaultColorsInit = true;
    }
}

void generate_polygon_vertices(Vec4 *vertices, int count, float y, int start_idx)
{
    for (int i = 0; i < count; i++)
    {
        float angle = (2.0f * PI * i) / count;
        vertices[start_idx + i] = {cosf(angle), y, sinf(angle), 1};
    }
}

Mesh make_polyhedron(int faceCount)
{
    ensureDefaultColors();
    Mesh cube = {cubeVertices, 8, cubeFaceCorners, cubeFaceCornerCounts, 6, defaultColors};
    Mesh tetrahedron = {tetrahedronVertices, 4, tetrahedronFaceCorners, tetrahedronFaceCornerCounts, 4, defaultColors};
    Mesh octahedron = {octahedronVertices, 6, octahedronFaceCorners, octahedronFaceCornerCounts, 8, defaultColors};
    Mesh icosahedron = {icosahedronVertices,         12, icosahedronFaceCorners,
                        icosahedronFaceCornerCounts, 20, defaultColors};
    Mesh dodecahedron = {dodecahedronVertices,         20, dodecahedronFaceCorners,
                         dodecahedronFaceCornerCounts, 12, defaultColors};

    switch (faceCount)
    {
    case 4:
        return tetrahedron;
    case 6:
        return cube;
    case 8:
        return octahedron;
    case 12:
        return dodecahedron;
    case 20:
        return icosahedron;
    default:
        printf("E: no polyhedron with %d faces (valid: 4, 6, 8, 12, 20)\n", faceCount);
        return {};
    }
}

Mesh make_pyramid(int sides)
{
    if (sides < 3)
    {
        sides = 3;
    }
    if (sides > MAX_PYRAMID_SIDES)
    {
        sides = MAX_PYRAMID_SIDES;
    }

    int vtxIdx = 0;
    int cornerIdx = 0;
    int faceIdx = 0;
    int colorIdx = 0;

    // base center and apex
    pyramidVertices[vtxIdx++] = {0, 0, 0, 1};
    pyramidVertices[vtxIdx++] = {0, 2, 0, 1};

    generate_polygon_vertices(pyramidVertices, sides, 0, vtxIdx);
    vtxIdx += sides;

    // base n-gon (single face)
    for (int i = 0; i < sides; i++)
    {
        pyramidFaceCorners[cornerIdx++] = 2 + i;
    }
    pyramidFaceCornerCounts[faceIdx++] = sides;
    pyramidColors[colorIdx++] = {255, 255, 255, 255};

    // side triangles
    for (int i = 0; i < sides; i++)
    {
        int next = (i + 1) % sides;
        pyramidFaceCorners[cornerIdx++] = 2 + i;    // base current
        pyramidFaceCorners[cornerIdx++] = 2 + next; // base next
        pyramidFaceCorners[cornerIdx++] = 1;        // apex
        pyramidFaceCornerCounts[faceIdx++] = 3;
        pyramidColors[colorIdx++] = {255, 255, 255, 255};
    }

    return {pyramidVertices, vtxIdx, pyramidFaceCorners, pyramidFaceCornerCounts, faceIdx, pyramidColors};
}

Mesh make_cylinder(int segments)
{
    if (segments < 3)
    {
        segments = 3;
    }
    if (segments > MAX_CYLINDER_SEGMENTS)
    {
        segments = MAX_CYLINDER_SEGMENTS;
    }

    int vtxIdx = 0;
    int cornerIdx = 0;
    int faceIdx = 0;
    int colorIdx = 0;

    // center vertices for caps
    cylinderVertices[vtxIdx++] = {0, 1, 0, 1};
    cylinderVertices[vtxIdx++] = {0, -1, 0, 1};

    // generate circle vertices (top and bottom)
    for (int i = 0; i < segments; i++)
    {
        float angle = (2.0f * PI * i) / segments;
        cylinderVertices[vtxIdx++] = {cosf(angle), 1, sinf(angle), 1};
        cylinderVertices[vtxIdx++] = {cosf(angle), -1, sinf(angle), 1};
    }

    // top cap n-gon (single face)
    for (int i = 0; i < segments; i++)
    {
        cylinderFaceCorners[cornerIdx++] = 2 + i * 2;
    }
    cylinderFaceCornerCounts[faceIdx++] = segments;
    cylinderColors[colorIdx++] = {255, 255, 255, 255};

    // bottom cap n-gon (single face)
    for (int i = 0; i < segments; i++)
    {
        cylinderFaceCorners[cornerIdx++] = 2 + i * 2 + 1;
    }
    cylinderFaceCornerCounts[faceIdx++] = segments;
    cylinderColors[colorIdx++] = {255, 255, 255, 255};

    // side quad faces (1 per segment)
    for (int i = 0; i < segments; i++)
    {
        int next = (i + 1) % segments;
        int topCurr = 2 + i * 2;
        int botCurr = 2 + i * 2 + 1;
        int topNext = 2 + next * 2;
        int botNext = 2 + next * 2 + 1;

        cylinderFaceCorners[cornerIdx++] = topCurr;
        cylinderFaceCorners[cornerIdx++] = botCurr;
        cylinderFaceCorners[cornerIdx++] = botNext;
        cylinderFaceCorners[cornerIdx++] = topNext;
        cylinderFaceCornerCounts[faceIdx++] = 4;
        cylinderColors[colorIdx++] = {255, 255, 255, 255};
    }

    return {cylinderVertices, vtxIdx, cylinderFaceCorners, cylinderFaceCornerCounts, faceIdx, cylinderColors};
}

} // namespace mesh
