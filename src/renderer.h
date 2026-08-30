#ifndef RENDERER_H
#define RENDERER_H

#include "buffer.h"

struct Renderer
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint commandBuffer = 0;
    GLuint modelBuffer = 0;
    GLuint faceColorBuffer = 0;
    GLuint faceOffsetBuffer = 0;

    GLenum primitive = GL_TRIANGLES;
    int slotCount = 0;
};

namespace render
{

void init(Renderer &r, GLuint idxVertex);
void upload_mesh(const Renderer &r, const Buffer &buf);
void upload_commands(Renderer &r, const Buffer &buf);
void upload_models(const Renderer &r, const Buffer &buf);
void upload_face_colors(const Renderer &r, const Buffer &buf, int faceVtxCount);
void draw(const Renderer &r);
void free(Renderer &r);

} // namespace render

#endif // RENDERER_H
