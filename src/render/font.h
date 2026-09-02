#ifndef FONT_H
#define FONT_H

// TODO: make it pretty

#include <stdint.h>

struct Glyph
{
    float u0, v0, u1, v1;
    float advance;
    float xoff;
    float yoff;
};

struct Font
{
    Glyph glyphs[95]; // ASCII 32..126
    int atlasWidth;
    int atlasHeight;
    int glyphWidth;
    int glyphHeight;
    int lineHeight;
};

const Font &baked_font();
const uint8_t *baked_atlas();

#endif // FONT_H
