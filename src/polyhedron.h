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
inline unsigned int cubeIndices[] = {
    0, 1, 2, //
    2, 1, 3, //
    6, 7, 4, //
    4, 7, 5, //
    1, 5, 3, //
    3, 5, 7, //
    0, 2, 4, //
    4, 2, 6, //
    0, 4, 1, //
    1, 4, 5, //
    2, 3, 6, //
    6, 3, 7,
};

inline Vec4 tetrahedronVertices[] = {
    {1, 1, 1, 1},   //
    {1, -1, -1, 1}, //
    {-1, 1, -1, 1}, //
    {-1, -1, 1, 1},
};
inline unsigned int tetrahedronIndices[] = {
    1, 3, 2, //
    0, 2, 3, //
    0, 3, 1, //
    0, 1, 2,
};

inline Vec4 octahedronVertices[] = {
    {1, 0, 0, 1},  //
    {-1, 0, 0, 1}, //
    {0, 1, 0, 1},  //
    {0, -1, 0, 1}, //
    {0, 0, 1, 1},  //
    {0, 0, -1, 1},
};
inline unsigned int octahedronIndices[] = {
    0, 2, 4, //
    0, 5, 2, //
    0, 4, 3, //
    0, 3, 5, //
    1, 4, 2, //
    1, 2, 5, //
    1, 3, 4, //
    1, 5, 3,
};

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
inline unsigned int icosahedronIndices[] = {
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
inline unsigned int dodecahedronIndices[] = {
    8,  10, 2,  //
    8,  2,  16, //
    8,  16, 0,  //
    11, 9,  1,  //
    11, 1,  17, //
    11, 17, 3,  //
    4,  18, 6,  //
    4,  6,  10, //
    4,  10, 8,  //
    7,  19, 5,  //
    7,  5,  9,  //
    7,  9,  11, //
    14, 4,  8,  //
    14, 8,  0,  //
    14, 0,  12, //
    5,  14, 12, //
    5,  12, 1,  //
    5,  1,  9,  //
    6,  15, 13, //
    6,  13, 2,  //
    6,  2,  10, //
    15, 7,  11, //
    15, 11, 3,  //
    15, 3,  13, //
    17, 1,  12, //
    17, 12, 0,  //
    17, 0,  16, //
    3,  17, 16, //
    3,  16, 2,  //
    3,  2,  13, //
    5,  19, 18, //
    5,  18, 4,  //
    5,  4,  14, //
    19, 7,  15, //
    19, 15, 6,  //
    19, 6,  18,
};

} // namespace mesh

#endif // POLYHEDRON_H
