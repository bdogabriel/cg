#ifndef DRAW_2D_H
#define DRAW_2D_H

#include "font.h"
#include "quad_batch.h"

namespace draw2d
{
void rect(QuadBatch &batch, float x, float y, float w, float h, Color color);
void rect_outline(QuadBatch &batch, float x, float y, float w, float h, Color color);
void text(QuadBatch &batch, const Font &font, const char *str, float x, float y, Color color, float scale = 1.0f);
} // namespace draw2d

#endif
