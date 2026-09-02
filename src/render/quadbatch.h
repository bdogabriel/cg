#ifndef QUADBATCH_H
#define QUADBATCH_H

#include "color.h"

struct Quad
{
    float x, y;
    float w, h;
    float u0, v0;
    float u1, v1;
    Color color;
};

namespace quad
{
constexpr int MAX_QUADS = 4096;
constexpr int MAX_VERTICES = MAX_QUADS * 4;
constexpr int MAX_INDICES = MAX_QUADS * 6;
} // namespace quad

struct QuadBatch
{
    float positions[2 * quad::MAX_VERTICES] = {};
    float uvs[2 * quad::MAX_VERTICES] = {};
    Color colors[quad::MAX_VERTICES] = {};
    unsigned int indices[quad::MAX_INDICES] = {};
    int vertexCount = 0;
    int indexCount = 0;
};

namespace quad
{
void reset(QuadBatch &batch);
void add(QuadBatch &batch, const Quad &q);
} // namespace quad

#endif
