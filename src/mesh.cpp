#include "mesh.h"

#include <stdio.h>

namespace mesh
{

static Vec4 pyramidVertices[MAX_PYRAMID_SIDES + 2];
static unsigned int pyramidIndices[MAX_PYRAMID_SIDES * 6];
static Color pyramidColors[MAX_PYRAMID_SIDES * 2];

static Vec4 cylinderVertices[MAX_CYLINDER_SEGMENTS * 2 + 2];
static unsigned int cylinderIndices[MAX_CYLINDER_SEGMENTS * 12];
static Color cylinderColors[MAX_CYLINDER_SEGMENTS * 4];

static Vec4 axisXVertices[] = {{0, 0, 0, 1}, {1, 0, 0, 1}};
static unsigned int axisXIndices[] = {0, 1};
static Color axisXColors[] = {color::palette[0]};

static Vec4 axisYVertices[] = {{0, 0, 0, 1}, {0, 1, 0, 1}};
static unsigned int axisYIndices[] = {0, 1};
static Color axisYColors[] = {color::palette[1]};

static Vec4 axisZVertices[] = {{0, 0, 0, 1}, {0, 0, 1, 1}};
static unsigned int axisZIndices[] = {0, 1};
static Color axisZColors[] = {color::palette[2]};

static Color cubeColors[] = {
    color::palette[0], color::palette[0], //
    color::palette[1], color::palette[1], //
    color::palette[2], color::palette[2], //
    color::palette[3], color::palette[3], //
    color::palette[4], color::palette[4], //
    color::palette[5], color::palette[5], //
};
static Color tetrahedronColors[] = {
    color::palette[0], //
    color::palette[1], //
    color::palette[2], //
    color::palette[3], //
};
static Color octahedronColors[] = {
    color::palette[0], //
    color::palette[1], //
    color::palette[2], //
    color::palette[3], //
    color::palette[4], //
    color::palette[5], //
    color::palette[6], //
    color::palette[7], //
};
static Color icosahedronColors[] = {
    color::palette[0], //
    color::palette[1], //
    color::palette[2], //
    color::palette[3], //
    color::palette[4], //
    color::palette[5], //
    color::palette[6], //
    color::palette[7], //
    color::palette[0], //
    color::palette[1], //
    color::palette[2], //
    color::palette[3], //
    color::palette[4], //
    color::palette[5], //
    color::palette[6], //
    color::palette[7], //
    color::palette[0], //
    color::palette[1], //
    color::palette[2], //
    color::palette[3], //
};
static Color dodecahedronColors[] = {
    color::palette[0], color::palette[0], color::palette[0], //
    color::palette[1], color::palette[1], color::palette[1], //
    color::palette[2], color::palette[2], color::palette[2], //
    color::palette[3], color::palette[3], color::palette[3], //
    color::palette[4], color::palette[4], color::palette[4], //
    color::palette[5], color::palette[5], color::palette[5], //
    color::palette[6], color::palette[6], color::palette[6], //
    color::palette[7], color::palette[7], color::palette[7], //
    color::palette[0], color::palette[0], color::palette[0], //
    color::palette[1], color::palette[1], color::palette[1], //
    color::palette[2], color::palette[2], color::palette[2], //
    color::palette[3], color::palette[3], color::palette[3], //
};

void generate_polygon_vertices(Vec4 *vertices, int count, float y, int start_idx)
{
    for (int i = 0; i < count; i++)
    {
        float angle = (2.0f * PI * i) / count;
        vertices[start_idx + i] = {cosf(angle), y, sinf(angle), 1};
    }
}

Mesh make_axis(const char axis)
{
    Mesh axisX = {axisXVertices, 2, axisXIndices, 2, axisXColors, 1, 2};
    Mesh axisY = {axisYVertices, 2, axisYIndices, 2, axisYColors, 1, 2};
    Mesh axisZ = {axisZVertices, 2, axisZIndices, 2, axisZColors, 1, 2};

    switch (axis)
    {
    case 'y':
        return axisY;
    case 'z':
        return axisZ;
    default:
        return axisX;
    }
}

Mesh make_polyhedron(int faceCount)
{
    Mesh cube = {cubeVertices, 8, cubeIndices, 36, cubeColors, 12, 3};
    Mesh tetrahedron = {tetrahedronVertices, 4, tetrahedronIndices, 12, tetrahedronColors, 4, 3};
    Mesh octahedron = {octahedronVertices, 6, octahedronIndices, 24, octahedronColors, 8, 3};
    Mesh icosahedron = {icosahedronVertices, 12, icosahedronIndices, 60, icosahedronColors, 20, 3};
    Mesh dodecahedron = {dodecahedronVertices, 20, dodecahedronIndices, 36, dodecahedronColors, 36, 3};

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
    int idxIdx = 0;
    int colorIdx = 0;

    // base center and apex
    pyramidVertices[vtxIdx++] = {0, 0, 0, 1};
    pyramidVertices[vtxIdx++] = {0, 2, 0, 1};

    generate_polygon_vertices(pyramidVertices, sides, 0, vtxIdx);
    vtxIdx += sides;

    // base triangles
    for (int i = 0; i < sides; i++)
    {
        int next = (i + 1) % sides;
        pyramidIndices[idxIdx++] = 0;        // base center
        pyramidIndices[idxIdx++] = 2 + next; // next vertex
        pyramidIndices[idxIdx++] = 2 + i;    // current vertex
        pyramidColors[colorIdx++] = color::palette[0];
    }

    // side triangles
    for (int i = 0; i < sides; i++)
    {
        int next = (i + 1) % sides;
        pyramidIndices[idxIdx++] = 2 + i;    // base current
        pyramidIndices[idxIdx++] = 2 + next; // base next
        pyramidIndices[idxIdx++] = 1;        // apex
        pyramidColors[colorIdx++] = color::palette[(i % (color::paletteSize - 1)) + 1];
    }

    return {pyramidVertices, vtxIdx, pyramidIndices, idxIdx, pyramidColors, colorIdx, 3};
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
    int idxIdx = 0;
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

    // top cap triangles
    for (int i = 0; i < segments; i++)
    {
        int next = (i + 1) % segments;
        cylinderIndices[idxIdx++] = 0;            // top center
        cylinderIndices[idxIdx++] = 2 + i * 2;    // current top
        cylinderIndices[idxIdx++] = 2 + next * 2; // next top
        cylinderColors[colorIdx++] = color::palette[0];
    }

    // bottom cap triangles
    for (int i = 0; i < segments; i++)
    {
        int next = (i + 1) % segments;
        cylinderIndices[idxIdx++] = 1;                // bottom center
        cylinderIndices[idxIdx++] = 2 + next * 2 + 1; // next bottom
        cylinderIndices[idxIdx++] = 2 + i * 2 + 1;    // current bottom
        cylinderColors[colorIdx++] = color::palette[1];
    }

    // side triangles (2 per segment)
    for (int i = 0; i < segments; i++)
    {
        int next = (i + 1) % segments;
        int topCurr = 2 + i * 2;
        int botCurr = 2 + i * 2 + 1;
        int topNext = 2 + next * 2;
        int botNext = 2 + next * 2 + 1;

        // first triangle
        cylinderIndices[idxIdx++] = topCurr;
        cylinderIndices[idxIdx++] = botCurr;
        cylinderIndices[idxIdx++] = topNext;
        cylinderColors[colorIdx++] = color::palette[(i % (color::paletteSize - 2)) + 2];

        // second triangle
        cylinderIndices[idxIdx++] = topNext;
        cylinderIndices[idxIdx++] = botCurr;
        cylinderIndices[idxIdx++] = botNext;
        cylinderColors[colorIdx++] = color::palette[(i % (color::paletteSize - 2)) + 2];
    }

    return {cylinderVertices, vtxIdx, cylinderIndices, idxIdx, cylinderColors, colorIdx, 3};
}

} // namespace mesh
