#include "draw_3d.h"

#include "arena.h"

#include <stdio.h>

namespace draw3d
{

constexpr float GOLDEN_RATIO = 1.6180339887f;
constexpr float GOLDEN_RATIO_INV = 0.6180339887f;

namespace
{

Vec4 cubeVertices[] = {
    {1, -1, 1, 1},   //
    {1, 1, 1, 1},    //
    {-1, -1, 1, 1},  //
    {-1, 1, 1, 1},   //
    {1, -1, -1, 1},  //
    {1, 1, -1, 1},   //
    {-1, -1, -1, 1}, //
    {-1, 1, -1, 1},
};
unsigned int cubeFaceCorners[] = {
    0, 1, 3, 2, // front (z=1)
    4, 5, 7, 6, // back (z=-1)
    1, 5, 7, 3, // top (y=1)
    0, 2, 6, 4, // bottom (y=-1)
    0, 1, 5, 4, // right (x=1)
    2, 3, 7, 6, // left (x=-1)
};
int cubeFaceCornerCounts[] = {4, 4, 4, 4, 4, 4};

Vec4 tetrahedronVertices[] = {
    {1, 1, 1, 1},   //
    {1, -1, -1, 1}, //
    {-1, 1, -1, 1}, //
    {-1, -1, 1, 1},
};
unsigned int tetrahedronFaceCorners[] = {
    1, 3, 2, //
    0, 2, 3, //
    0, 3, 1, //
    0, 1, 2,
};
int tetrahedronFaceCornerCounts[] = {3, 3, 3, 3};

Vec4 octahedronVertices[] = {
    {1, 0, 0, 1},  //
    {-1, 0, 0, 1}, //
    {0, 1, 0, 1},  //
    {0, -1, 0, 1}, //
    {0, 0, 1, 1},  //
    {0, 0, -1, 1},
};
unsigned int octahedronFaceCorners[] = {
    0, 2, 4, //
    0, 5, 2, //
    0, 4, 3, //
    0, 3, 5, //
    1, 4, 2, //
    1, 2, 5, //
    1, 3, 4, //
    1, 5, 3,
};
int octahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3};

Vec4 icosahedronVertices[] = {
    {-1, GOLDEN_RATIO, 0, 1},  //
    {1, GOLDEN_RATIO, 0, 1},   //
    {-1, -GOLDEN_RATIO, 0, 1}, //
    {1, -GOLDEN_RATIO, 0, 1},  //
    {0, -1, GOLDEN_RATIO, 1},  //
    {0, 1, GOLDEN_RATIO, 1},   //
    {0, -1, -GOLDEN_RATIO, 1}, //
    {0, 1, -GOLDEN_RATIO, 1},  //
    {GOLDEN_RATIO, 0, -1, 1},  //
    {GOLDEN_RATIO, 0, 1, 1},   //
    {-GOLDEN_RATIO, 0, -1, 1}, //
    {-GOLDEN_RATIO, 0, 1, 1},
};
unsigned int icosahedronFaceCorners[] = {
    0,  11, 5,  //
    0,  5,  1,  //
    0,  1,  7,  //
    0,  7,  10, //
    0,  10, 11, //
    1,  5,  9,  //
    5,  11, 4,  //
    11, 10, 2,  //
    10, 7,  6,  //
    7,  1,  8,  //
    3,  9,  4,  //
    3,  4,  2,  //
    3,  2,  6,  //
    3,  6,  8,  //
    3,  8,  9,  //
    4,  9,  5,  //
    2,  4,  11, //
    6,  2,  10, //
    8,  6,  7,  //
    9,  8,  1,
};
int icosahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

Vec4 dodecahedronVertices[] = {
    {1, 1, 1, 1},                             //
    {1, 1, -1, 1},                            //
    {1, -1, 1, 1},                            //
    {1, -1, -1, 1},                           //
    {-1, 1, 1, 1},                            //
    {-1, 1, -1, 1},                           //
    {-1, -1, 1, 1},                           //
    {-1, -1, -1, 1},                          //
    {0, GOLDEN_RATIO_INV, GOLDEN_RATIO, 1},   //
    {0, GOLDEN_RATIO_INV, -GOLDEN_RATIO, 1},  //
    {0, -GOLDEN_RATIO_INV, GOLDEN_RATIO, 1},  //
    {0, -GOLDEN_RATIO_INV, -GOLDEN_RATIO, 1}, //
    {GOLDEN_RATIO_INV, GOLDEN_RATIO, 0, 1},   //
    {GOLDEN_RATIO_INV, -GOLDEN_RATIO, 0, 1},  //
    {-GOLDEN_RATIO_INV, GOLDEN_RATIO, 0, 1},  //
    {-GOLDEN_RATIO_INV, -GOLDEN_RATIO, 0, 1}, //
    {GOLDEN_RATIO, 0, GOLDEN_RATIO_INV, 1},   //
    {GOLDEN_RATIO, 0, -GOLDEN_RATIO_INV, 1},  //
    {-GOLDEN_RATIO, 0, GOLDEN_RATIO_INV, 1},  //
    {-GOLDEN_RATIO, 0, -GOLDEN_RATIO_INV, 1},
};
unsigned int dodecahedronFaceCorners[] = {
    8,  10, 2,  16, 0,  // pentagon 1
    11, 9,  1,  17, 3,  // pentagon 2
    4,  18, 6,  10, 8,  // pentagon 3
    7,  19, 5,  9,  11, // pentagon 4
    14, 4,  8,  0,  12, // pentagon 5
    5,  14, 12, 1,  9,  // pentagon 6
    6,  15, 13, 2,  10, // pentagon 7
    15, 7,  11, 3,  13, // pentagon 8
    17, 1,  12, 0,  16, // pentagon 9
    3,  17, 16, 2,  13, // pentagon 10
    5,  19, 18, 4,  14, // pentagon 11
    19, 7,  15, 6,  18, // pentagon 12
};
int dodecahedronFaceCornerCounts[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

Color defaultColors[20];
bool defaultColorsInit = false;

void ensureDefaultColors()
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
        float angle = (2.0f * mesh::PI * i) / count;
        vertices[start_idx + i] = {cosf(angle), y, sinf(angle), 1};
    }
}

} // namespace

Mesh polyhedron(int faceCount)
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

Mesh pyramid(int sides)
{
    if (sides < 3)
    {
        sides = 3;
    }

    int vtxCount = sides + 2;
    int faceCount = sides + 1;
    int cornerCount = sides + sides * 3;

    Vec4 *vertices = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)vtxCount);
    unsigned int *faceCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)cornerCount);
    int *faceCornerCounts = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    Color *colors = (Color *)arena::scratch_alloc(sizeof(Color) * (size_t)faceCount);

    int vtxIdx = 0;
    int cornerIdx = 0;
    int faceIdx = 0;

    // base center and apex
    vertices[vtxIdx++] = {0, 0, 0, 1};
    vertices[vtxIdx++] = {0, 2, 0, 1};

    generate_polygon_vertices(vertices, sides, 0, vtxIdx);
    vtxIdx += sides;

    // base n-gon (single face)
    for (int i = 0; i < sides; i++)
    {
        faceCorners[cornerIdx++] = 2 + i;
    }
    faceCornerCounts[faceIdx] = sides;
    colors[faceIdx] = {255, 255, 255, 255};
    faceIdx++;

    // side triangles
    for (int i = 0; i < sides; i++)
    {
        int next = (i + 1) % sides;
        faceCorners[cornerIdx++] = 2 + i;    // base current
        faceCorners[cornerIdx++] = 2 + next; // base next
        faceCorners[cornerIdx++] = 1;        // apex
        faceCornerCounts[faceIdx] = 3;
        colors[faceIdx] = {255, 255, 255, 255};
        faceIdx++;
    }

    return {vertices, vtxIdx, faceCorners, faceCornerCounts, faceIdx, colors};
}

Mesh cylinder(int segments)
{
    if (segments < 3)
    {
        segments = 3;
    }

    int vtxCount = segments * 2 + 2;
    int faceCount = segments + 2;
    int cornerCount = segments * 6;

    Vec4 *vertices = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)vtxCount);
    unsigned int *faceCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)cornerCount);
    int *faceCornerCounts = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    Color *colors = (Color *)arena::scratch_alloc(sizeof(Color) * (size_t)faceCount);

    int vtxIdx = 0;
    int cornerIdx = 0;
    int faceIdx = 0;

    // center vertices for caps
    vertices[vtxIdx++] = {0, 1, 0, 1};
    vertices[vtxIdx++] = {0, -1, 0, 1};

    // generate circle vertices (top and bottom)
    for (int i = 0; i < segments; i++)
    {
        float angle = (2.0f * mesh::PI * i) / segments;
        vertices[vtxIdx++] = {cosf(angle), 1, sinf(angle), 1};
        vertices[vtxIdx++] = {cosf(angle), -1, sinf(angle), 1};
    }

    // top cap n-gon (single face)
    for (int i = 0; i < segments; i++)
    {
        faceCorners[cornerIdx++] = 2 + i * 2;
    }
    faceCornerCounts[faceIdx] = segments;
    colors[faceIdx] = {255, 255, 255, 255};
    faceIdx++;

    // bottom cap n-gon (single face)
    for (int i = 0; i < segments; i++)
    {
        faceCorners[cornerIdx++] = 2 + i * 2 + 1;
    }
    faceCornerCounts[faceIdx] = segments;
    colors[faceIdx] = {255, 255, 255, 255};
    faceIdx++;

    // side quad faces (1 per segment)
    for (int i = 0; i < segments; i++)
    {
        int next = (i + 1) % segments;
        int topCurr = 2 + i * 2;
        int botCurr = 2 + i * 2 + 1;
        int topNext = 2 + next * 2;
        int botNext = 2 + next * 2 + 1;

        faceCorners[cornerIdx++] = topCurr;
        faceCorners[cornerIdx++] = botCurr;
        faceCorners[cornerIdx++] = botNext;
        faceCorners[cornerIdx++] = topNext;
        faceCornerCounts[faceIdx] = 4;
        colors[faceIdx] = {255, 255, 255, 255};
        faceIdx++;
    }

    return {vertices, vtxIdx, faceCorners, faceCornerCounts, faceIdx, colors};
}

Mesh sphere(int segments)
{
    if (segments < 3)
    {
        segments = 3;
    }

    int slices = segments;
    int stacks = segments;

    int vtxCount = 2 + (stacks - 1) * slices;
    int faceCount = slices * stacks;
    int cornerCount = 4 * slices * stacks - 2 * slices;

    Vec4 *vertices = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)vtxCount);
    unsigned int *faceCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)cornerCount);
    int *faceCornerCounts = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    Color *colors = (Color *)arena::scratch_alloc(sizeof(Color) * (size_t)faceCount);

    int vtxIdx = 0;
    int cornerIdx = 0;
    int faceIdx = 0;

    // north and south poles
    vertices[vtxIdx++] = {0, 1, 0, 1};
    vertices[vtxIdx++] = {0, -1, 0, 1};

    // intermediate latitude rings (ring 1 .. stacks-1)
    for (int i = 1; i < stacks; i++)
    {
        float theta = (mesh::PI * i) / stacks;
        float y = cosf(theta);
        float r = sinf(theta);
        for (int j = 0; j < slices; j++)
        {
            float phi = (2.0f * mesh::PI * j) / slices;
            vertices[vtxIdx++] = {r * cosf(phi), y, r * sinf(phi), 1};
        }
    }

    // ring i base vertex index (ring 0 = north pole, ring stacks = south pole)
    auto ringBase = [&](int ring) { return 2 + (ring - 1) * slices; };

    // top cap: triangle fans from the north pole to ring 1
    for (int j = 0; j < slices; j++)
    {
        faceCorners[cornerIdx++] = 0;
        faceCorners[cornerIdx++] = ringBase(1) + j;
        faceCorners[cornerIdx++] = ringBase(1) + (j + 1) % slices;
        faceCornerCounts[faceIdx] = 3;
        colors[faceIdx] = {255, 255, 255, 255};
        faceIdx++;
    }

    // body: quads between consecutive rings
    for (int i = 1; i < stacks - 1; i++)
    {
        for (int j = 0; j < slices; j++)
        {
            int next = (j + 1) % slices;
            faceCorners[cornerIdx++] = ringBase(i) + j;
            faceCorners[cornerIdx++] = ringBase(i) + next;
            faceCorners[cornerIdx++] = ringBase(i + 1) + next;
            faceCorners[cornerIdx++] = ringBase(i + 1) + j;
            faceCornerCounts[faceIdx] = 4;
            colors[faceIdx] = {255, 255, 255, 255};
            faceIdx++;
        }
    }

    // bottom cap: triangle fans from the south pole to ring stacks-1
    for (int j = 0; j < slices; j++)
    {
        faceCorners[cornerIdx++] = 1;
        faceCorners[cornerIdx++] = ringBase(stacks - 1) + (j + 1) % slices;
        faceCorners[cornerIdx++] = ringBase(stacks - 1) + j;
        faceCornerCounts[faceIdx] = 3;
        colors[faceIdx] = {255, 255, 255, 255};
        faceIdx++;
    }

    return {vertices, vtxIdx, faceCorners, faceCornerCounts, faceIdx, colors};
}

} // namespace draw3d
