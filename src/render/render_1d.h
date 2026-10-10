#ifndef RENDER_1D_H
#define RENDER_1D_H

#include <GL/glew.h>

#include "line_batch.h"

struct Render1D
{
    GLuint vao = 0;
    GLuint posVbo = 0;
    GLuint colVbo = 0;
};

namespace render1d
{
void init(Render1D &r);
void draw(Render1D &r, const LineBatch &batch);
void free(Render1D &r);
} // namespace render1d

#endif
