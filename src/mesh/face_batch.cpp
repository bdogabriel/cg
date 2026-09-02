#include "face_batch.h"

namespace face
{

void update_corner_starts(FaceBatch &fb)
{
    int running = 0;
    for (int i = 0; i < fb.faceCount; i++)
    {
        fb.faceCornerStarts[i] = running;
        running += fb.faceCornerCounts[i];
    }
}

} // namespace face
