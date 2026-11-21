#ifndef RENDER_3D_H
#define RENDER_3D_H

#include <GL/glew.h>

#include "mesh_batch.h"
#include "model_texture_cache.h"

struct Render3D
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint uvVbo = 0;
    GLuint ebo = 0;
    GLuint commandBuffer = 0;
    GLuint modelBuffer = 0;
    GLuint faceColorBuffer = 0;
    GLuint faceOffsetBuffer = 0;

    GLenum primitive = GL_TRIANGLES;
};

namespace render3d
{

void init(Render3D &r, GLuint idxVertex);
void upload_mesh(const Render3D &r, const MeshBatch &batch);
void upload_commands(Render3D &r, const MeshBatch &batch);
void upload_models(const Render3D &r, const MeshBatch &batch);
void upload_face_colors(const Render3D &r, const MeshBatch &batch);

void draw_flat(const Render3D &r, const MeshBatch &batch);
void draw_textured(const Render3D &r, const MeshBatch &batch, const ModelTextureCache &modelTexCache);
void free(Render3D &r);

} // namespace render3d

#endif // RENDER_3D_H
