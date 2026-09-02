#ifndef LINE_RENDER_H
#define LINE_RENDER_H

#include <GL/glew.h>

#include "linebatch.h"

struct Render1d
{
    GLuint vao = 0;
    GLuint posVbo = 0;
    GLuint colVbo = 0;
};

namespace render1d
{
void init(Render1d &r);
void draw(Render1d &r, const LineBatch &batch);
void free(Render1d &r);
} // namespace render1d

#endif
