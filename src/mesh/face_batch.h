#ifndef FACE_BATCH_H
#define FACE_BATCH_H

#include "color.h"
#include "mesh.h"

namespace face
{
constexpr int MAX_FACES = 8000;
constexpr int MAX_CORNERS = 24000;
} // namespace face

struct FaceBatch
{
    unsigned int faceCorners[face::MAX_CORNERS] = {};
    int faceCornerStarts[face::MAX_FACES] = {};
    int faceCornerCounts[face::MAX_FACES] = {};
    Color faceColors[face::MAX_FACES] = {};
    int cornerCount = 0;
    int faceCount = 0;
    int faceOffsets[mesh::MAX_REFS] = {};
    int faceCounts[mesh::MAX_REFS] = {};
};

namespace face
{
void update_corner_starts(FaceBatch &fb);
} // namespace face

#endif // FACE_BATCH_H
