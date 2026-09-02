#include "quadbatch.h"

namespace quad
{

void reset(QuadBatch &batch)
{
    batch.vertexCount = 0;
    batch.indexCount = 0;
}

void add(QuadBatch &batch, const Quad &q)
{
    if (batch.vertexCount + 4 > quad::MAX_VERTICES || batch.indexCount + 6 > quad::MAX_INDICES)
    {
        return;
    }

    int base = batch.vertexCount;

    batch.positions[batch.vertexCount * 2 + 0] = q.x;
    batch.positions[batch.vertexCount * 2 + 1] = q.y;
    batch.uvs[batch.vertexCount * 2 + 0] = q.u0;
    batch.uvs[batch.vertexCount * 2 + 1] = q.v0;
    batch.colors[batch.vertexCount] = q.color;
    batch.vertexCount++;

    batch.positions[batch.vertexCount * 2 + 0] = q.x + q.w;
    batch.positions[batch.vertexCount * 2 + 1] = q.y;
    batch.uvs[batch.vertexCount * 2 + 0] = q.u1;
    batch.uvs[batch.vertexCount * 2 + 1] = q.v0;
    batch.colors[batch.vertexCount] = q.color;
    batch.vertexCount++;

    batch.positions[batch.vertexCount * 2 + 0] = q.x + q.w;
    batch.positions[batch.vertexCount * 2 + 1] = q.y + q.h;
    batch.uvs[batch.vertexCount * 2 + 0] = q.u1;
    batch.uvs[batch.vertexCount * 2 + 1] = q.v1;
    batch.colors[batch.vertexCount] = q.color;
    batch.vertexCount++;

    batch.positions[batch.vertexCount * 2 + 0] = q.x;
    batch.positions[batch.vertexCount * 2 + 1] = q.y + q.h;
    batch.uvs[batch.vertexCount * 2 + 0] = q.u0;
    batch.uvs[batch.vertexCount * 2 + 1] = q.v1;
    batch.colors[batch.vertexCount] = q.color;
    batch.vertexCount++;

    batch.indices[batch.indexCount++] = base + 0;
    batch.indices[batch.indexCount++] = base + 1;
    batch.indices[batch.indexCount++] = base + 2;
    batch.indices[batch.indexCount++] = base + 0;
    batch.indices[batch.indexCount++] = base + 2;
    batch.indices[batch.indexCount++] = base + 3;
}

} // namespace quad
