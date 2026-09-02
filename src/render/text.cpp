#include "text.h"

namespace text
{

void append_text(QuadBatch &batch, const Font &font, const char *str, float x, float y, Color color, float scale)
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

void fill_rect(QuadBatch &batch, const Font &font, float x, float y, float w, float h, Color color)
{
    float whiteU = (font.atlasWidth - 0.5f) / (float)font.atlasWidth;
    float whiteV = (font.atlasHeight - 0.5f) / (float)font.atlasHeight;
    Quad q;
    q.x = x;
    q.y = y;
    q.w = w;
    q.h = h;
    q.u0 = whiteU;
    q.v0 = whiteV;
    q.u1 = whiteU;
    q.v1 = whiteV;
    q.color = color;
    quad::add(batch, q);
}

void outline_rect(QuadBatch &batch, const Font &font, float x, float y, float w, float h, Color color)
{
    float thickness = 1.0f;
    fill_rect(batch, font, x, y, w, thickness, color);
    fill_rect(batch, font, x, y, thickness, h, color);
    fill_rect(batch, font, x + w - thickness, y, thickness, h, color);
    fill_rect(batch, font, x, y + h - thickness, w, thickness, color);
}

float measure(const Font &font, const char *str, float scale)
{
    float width = 0.0f;
    while (*str != '\0')
    {
        unsigned char c = static_cast<unsigned char>(*str);
        if (c < 32 || c > 126)
        {
            width += font.glyphWidth * scale;
        }
        else
        {
            width += font.glyphs[c - 32].advance * scale;
        }
        str++;
    }
    return width;
}

} // namespace text
