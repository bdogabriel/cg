#include "draw_3d.h"

#include <stdio.h>

namespace draw3d
{

constexpr float GOLDEN_RATIO = 1.6180339887f;
constexpr float GOLDEN_RATIO_INV = 0.6180339887f;

static Vec4 cubeVertices[] = {
    {1, -1, 1, 1},   //
    {1, 1, 1, 1},    //
    {-1, -1, 1, 1},  //
    {-1, 1, 1, 1},   //
    {1, -1, -1, 1},  //
    {1, 1, -1, 1},   //
    {-1, -1, -1, 1}, //
    {-1, 1, -1, 1},
};
static unsigned int cubeFaceCorners[] = {
    0, 1, 3, 2, // front (z=1)
    4, 5, 7, 6, // back (z=-1)
    1, 5, 7, 3, // top (y=1)
    0, 2, 6, 4, // bottom (y=-1)
    0, 1, 5, 4, // right (x=1)
    2, 3, 7, 6, // left (x=-1)
};
static int cubeFaceCornerCounts[] = {4, 4, 4, 4, 4, 4};

static Vec4 tetrahedronVertices[] = {
    {1, 1, 1, 1},   //
    {1, -1, -1, 1}, //
    {-1, 1, -1, 1}, //
    {-1, -1, 1, 1},
};
static unsigned int tetrahedronFaceCorners[] = {
    1, 3, 2, //
    0, 2, 3, //
    0, 3, 1, //
    0, 1, 2,
};
static int tetrahedronFaceCornerCounts[] = {3, 3, 3, 3};

static Vec4 octahedronVertices[] = {
    {1, 0, 0, 1},  //
    {-1, 0, 0, 1}, //
    {0, 1, 0, 1},  //
    {0, -1, 0, 1}, //
    {0, 0, 1, 1},  //
    {0, 0, -1, 1},
};
static unsigned int octahedronFaceCorners[] = {
    0, 2, 4, //
    0, 5, 2, //
    0, 4, 3, //
    0, 3, 5, //
    1, 4, 2, //
    1, 2, 5, //
    1, 3, 4, //
    1, 5, 3,
};
static int octahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3};

static Vec4 icosahedronVertices[] = {
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
static unsigned int icosahedronFaceCorners[] = {
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
static int icosahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

static Vec4 dodecahedronVertices[] = {
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
static unsigned int dodecahedronFaceCorners[] = {
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
static int dodecahedronFaceCornerCounts[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

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

static void generate_polygon_vertices(Vec4 *vertices, int count, float y, int start_idx)
{
    for (int i = 0; i < count; i++)
    {
        float angle = (2.0f * mesh::PI * i) / count;
        vertices[start_idx + i] = {cosf(angle), y, sinf(angle), 1};
    }
}

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

Mesh cylinder(int segments)
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
        float angle = (2.0f * mesh::PI * i) / segments;
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

} // namespace draw3d
