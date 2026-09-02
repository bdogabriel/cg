#ifndef RENDER2D_H
#define RENDER2D_H

#include "font.h"
#include "quadbatch.h"

#include <GL/glew.h>

struct Render2d
{
    GLuint vao = 0;
    GLuint posVbo = 0;
    GLuint uvVbo = 0;
    GLuint colVbo = 0;
    GLuint ebo = 0;
    GLuint atlas = 0;
};

namespace render2d
{
void init(Render2d &r, const Font &font, const uint8_t *atlas);
void draw(const Render2d &r, const QuadBatch &batch);
void free(Render2d &r);
} // namespace render2d

#endif // RENDER2D_H
