#ifndef FACE_BATCH_H
#define FACE_BATCH_H

#include "buffer.h"
#include "mat4.h"

struct FaceBatch
{
    Buffer faceCorners;      // unsigned int
    Buffer faceCornerStarts; // int
    Buffer faceCornerCounts; // int
    Buffer faceColors;       // Color
    Buffer faceMaterialIds;  // int
    int cornerCount = 0;
    int faceCount = 0;
    Buffer faceOffsets; // int, indexed by slot
    Buffer faceCounts;  // int, indexed by slot
};

namespace face
{
void update_corner_starts(FaceBatch &batch);
int triangulate(const Vec4 *loop, int count, int *outTris);
} // namespace face

#endif // FACE_BATCH_H
