#ifndef RENDER3D_H
#define RENDER3D_H

#include <GL/glew.h>

#include "meshbatch.h"

struct Render3d
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

namespace render3d
{

void init(Render3d &r, GLuint idxVertex);
void upload_mesh(const Render3d &r, const MeshBatch &buf);
void upload_commands(Render3d &r, const MeshBatch &buf);
void upload_models(const Render3d &r, const MeshBatch &buf);
void upload_face_colors(const Render3d &r, const MeshBatch &buf);

void draw(const Render3d &r);
void free(Render3d &r);

} // namespace render3d

#endif // RENDER3D_H
