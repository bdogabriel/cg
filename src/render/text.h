#ifndef TEXT_H
#define TEXT_H

#include "font.h"
#include "quadbatch.h"

namespace text
{
void append_text(QuadBatch &batch, const Font &font, const char *str, float x, float y, Color color, float scale = 1.0f);
void fill_rect(QuadBatch &batch, const Font &font, float x, float y, float w, float h, Color color);
void outline_rect(QuadBatch &batch, const Font &font, float x, float y, float w, float h, Color color);
} // namespace text

#endif
