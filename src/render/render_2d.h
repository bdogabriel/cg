#ifndef RENDER_2D_H
#define RENDER_2D_H

#include "quad_batch.h"

#include <GL/glew.h>

struct Render2D
{
    GLuint vao = 0;
    GLuint posVbo = 0;
    GLuint uvVbo = 0;
    GLuint colVbo = 0;
    GLuint ebo = 0;
};

namespace render2d
{
void init(Render2D &r);
void draw(const Render2D &r, const QuadBatch &batch);
void free(Render2D &r);
} // namespace render2d

#endif // RENDER2D_H
