#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int ATLAS_W = 512;
static const int ATLAS_H = 512;
static const int CELL_W = 16;
static const int CELL_H = 32;
static const int COLS = 32;

static void load_bmp_grayscale(const char *path, uint8_t *out, int width, int height)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "E: cannot open font atlas %s\n", path);
        exit(1);
    }

    unsigned char header[138];
    if (fread(header, 1, sizeof(header), f) != sizeof(header))
    {
        fprintf(stderr, "E: font atlas header too small\n");
        fclose(f);
        exit(1);
    }

    if (header[0] != 'B' || header[1] != 'M')
    {
        fprintf(stderr, "E: font atlas is not a BMP\n");
        fclose(f);
        exit(1);
    }

    int offBits = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
    int w = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    int h = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    int bpp = header[28] | (header[29] << 8);

    if (w != width || h != height || bpp != 32)
    {
        fprintf(stderr, "E: font atlas format mismatch (got %dx%d %dbpp, expected %dx%d 32bpp)\n", w, h, bpp, width,
                height);
        fclose(f);
        exit(1);
    }

    int rowSize = width * 4;
    unsigned char *row = (unsigned char *)malloc(rowSize);
    if (!row)
    {
        fprintf(stderr, "E: out of memory loading font atlas\n");
        fclose(f);
        exit(1);
    }

    for (int y = 0; y < height; y++)
    {
        int fileRow = height - 1 - y;
        fseek(f, offBits + fileRow * rowSize, SEEK_SET);
        if (fread(row, 1, rowSize, f) != (size_t)rowSize)
        {
            fprintf(stderr, "E: font atlas read error\n");
            free(row);
            fclose(f);
            exit(1);
        }
        for (int x = 0; x < width; x++)
        {
            out[y * width + x] = row[x * 4 + 2];
        }
    }

    free(row);
    fclose(f);
}

const Font &baked_font()
{
    static Font font;
    static bool initialized = false;
    if (!initialized)
    {
        font.atlasWidth = ATLAS_W;
        font.atlasHeight = ATLAS_H;
        font.glyphWidth = CELL_W;
        font.glyphHeight = CELL_H;
        font.lineHeight = CELL_H;
        for (int i = 0; i < 95; i++)
        {
            int code = 32 + i;
            int col = code % COLS;
            int row = code / COLS;
            font.glyphs[i].u0 = (col * CELL_W) / (float)ATLAS_W;
            font.glyphs[i].v0 = (row * CELL_H) / (float)ATLAS_H;
            font.glyphs[i].u1 = ((col + 1) * CELL_W) / (float)ATLAS_W;
            font.glyphs[i].v1 = ((row + 1) * CELL_H) / (float)ATLAS_H;
            font.glyphs[i].advance = CELL_W;
            font.glyphs[i].xoff = 0;
            font.glyphs[i].yoff = 0;
        }
        initialized = true;
    }
    return font;
}

const uint8_t *baked_atlas()
{
    static uint8_t atlas[ATLAS_W * ATLAS_H];
    static bool initialized = false;
    if (!initialized)
    {
        load_bmp_grayscale("assets/JetBrainsMono-Regular.bmp", atlas, ATLAS_W, ATLAS_H);
        atlas[ATLAS_W * ATLAS_H - 1] = 0xFF;
        initialized = true;
    }
    return atlas;
}
