#ifndef LINEBATCH_H
#define LINEBATCH_H

#include "color.h"
#include "mat4.h"

namespace line
{
constexpr int MAX_VERTICES = 16384;
} // namespace line

struct LineBatch
{
    Vec4 positions[line::MAX_VERTICES] = {};
    Color colors[line::MAX_VERTICES] = {};
    int vtxCount = 0;
};

namespace line
{
void reset(LineBatch &b);
bool can_add(const LineBatch &batch, int segments);
void add(LineBatch &batch, Vec4 a, Vec4 b, Color c);
} // namespace line

#endif
