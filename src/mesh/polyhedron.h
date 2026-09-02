#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include "mat4.h"

namespace mesh
{
constexpr float GOLDEN_RATIO = 1.6180339887f;
constexpr float GOLDEN_RATIO_INV = 0.6180339887f;

inline Vec4 cubeVertices[] = {
    {1, -1, 1, 1},   //
    {1, 1, 1, 1},    //
    {-1, -1, 1, 1},  //
    {-1, 1, 1, 1},   //
    {1, -1, -1, 1},  //
    {1, 1, -1, 1},   //
    {-1, -1, -1, 1}, //
    {-1, 1, -1, 1},
};
inline unsigned int cubeFaceCorners[] = {
    0, 1, 3, 2, // front (z=1)
    4, 5, 7, 6, // back (z=-1)
    1, 5, 7, 3, // top (y=1)
    0, 2, 6, 4, // bottom (y=-1)
    0, 1, 5, 4, // right (x=1)
    2, 3, 7, 6, // left (x=-1)
};
inline int cubeFaceCornerCounts[] = {4, 4, 4, 4, 4, 4};

inline Vec4 tetrahedronVertices[] = {
    {1, 1, 1, 1},   //
    {1, -1, -1, 1}, //
    {-1, 1, -1, 1}, //
    {-1, -1, 1, 1},
};
inline unsigned int tetrahedronFaceCorners[] = {
    1, 3, 2, //
    0, 2, 3, //
    0, 3, 1, //
    0, 1, 2,
};
inline int tetrahedronFaceCornerCounts[] = {3, 3, 3, 3};

inline Vec4 octahedronVertices[] = {
    {1, 0, 0, 1},  //
    {-1, 0, 0, 1}, //
    {0, 1, 0, 1},  //
    {0, -1, 0, 1}, //
    {0, 0, 1, 1},  //
    {0, 0, -1, 1},
};
inline unsigned int octahedronFaceCorners[] = {
    0, 2, 4, //
    0, 5, 2, //
    0, 4, 3, //
    0, 3, 5, //
    1, 4, 2, //
    1, 2, 5, //
    1, 3, 4, //
    1, 5, 3,
};
inline int octahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3};

inline Vec4 icosahedronVertices[] = {
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
inline unsigned int icosahedronFaceCorners[] = {
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
inline int icosahedronFaceCornerCounts[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

inline Vec4 dodecahedronVertices[] = {
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
inline unsigned int dodecahedronFaceCorners[] = {
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
inline int dodecahedronFaceCornerCounts[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

} // namespace mesh

#endif // POLYHEDRON_H
