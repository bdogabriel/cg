#include "linebatch.h"

namespace line
{
void reset(LineBatch &batch)
{
    batch.vtxCount = 0;
}

bool can_add(const LineBatch &batch, int segments)
{
    return batch.vtxCount + segments * 2 <= line::MAX_VERTICES;
}

void add(LineBatch &batch, Vec4 a, Vec4 b, Color c)
{
    batch.positions[batch.vtxCount] = a;
    batch.colors[batch.vtxCount] = c;
    batch.vtxCount++;
    batch.positions[batch.vtxCount] = b;
    batch.colors[batch.vtxCount] = c;
    batch.vtxCount++;
}
} // namespace line
