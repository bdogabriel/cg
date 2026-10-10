#include "draw_2d.h"

namespace draw2d
{

void rect(QuadBatch &batch, float x, float y, float w, float h, Color color)
{
    Quad q = {};
    q.x = x;
    q.y = y;
    q.w = w;
    q.h = h;
    q.color = color;
    quad::add(batch, q);
}

void rect_outline(QuadBatch &batch, float x, float y, float w, float h, Color color)
{
    float thickness = 1.0f;
    rect(batch, x, y, w, thickness, color);
    rect(batch, x, y, thickness, h, color);
    rect(batch, x + w - thickness, y, thickness, h, color);
    rect(batch, x, y + h - thickness, w, thickness, color);
}

void text(QuadBatch &batch, const Font &font, const char *str, float x, float y, Color color, float scale)
{
    Color clamped = color;
    if (clamped.a == 0)
    {
        clamped.a = 255;
    }
    float px = x;
    while (*str != '\0')
    {
        unsigned char c = static_cast<unsigned char>(*str);
        if (c < 32 || c > 126)
        {
            str++;
            px += font.glyphWidth * scale;
            continue;
        }
        const Glyph &glyph = font.glyphs[c - 32];
        float gw = font.glyphWidth * scale;
        float gh = font.glyphHeight * scale;
        Quad q;
        q.x = px;
        q.y = y;
        q.w = gw;
        q.h = gh;
        q.u0 = glyph.u0;
        q.v0 = glyph.v0;
        q.u1 = glyph.u1;
        q.v1 = glyph.v1;
        q.color = clamped;
        quad::add(batch, q);
        px += glyph.advance * scale;
        str++;
    }
}

} // namespace draw2d
