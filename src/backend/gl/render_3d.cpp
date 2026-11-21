#include "render_3d.h"

namespace render3d
{

void init(Render3D &r, GLuint idxVertex)
{
    glCreateVertexArrays(1, &r.vao);

    glCreateBuffers(1, &r.vbo);
    glCreateBuffers(1, &r.uvVbo);
    glCreateBuffers(1, &r.ebo);
    glCreateBuffers(1, &r.commandBuffer);
    glCreateBuffers(1, &r.modelBuffer);
    glCreateBuffers(1, &r.faceColorBuffer);
    glCreateBuffers(1, &r.faceOffsetBuffer);

    glVertexArrayVertexBuffer(r.vao, 0, r.vbo, 0, sizeof(Vec4));
    glVertexArrayVertexBuffer(r.vao, 1, r.uvVbo, 0, sizeof(Vec2));
    glVertexArrayElementBuffer(r.vao, r.ebo);

    glEnableVertexArrayAttrib(r.vao, idxVertex);
    glVertexArrayAttribFormat(r.vao, idxVertex, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, idxVertex, 0);

    glEnableVertexArrayAttrib(r.vao, 1);
    glVertexArrayAttribFormat(r.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, 1, 1);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r.faceColorBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r.faceOffsetBuffer);
}

void upload_mesh(const Render3D &r, const MeshBatch &batch)
{
    glNamedBufferData(r.vbo, batch.vtxCount * sizeof(Vec4), BUF(Vec4, batch.vertices), GL_DYNAMIC_DRAW);
    glNamedBufferData(r.uvVbo, batch.vtxCount * sizeof(Vec2), BUF(Vec2, batch.uvs), GL_DYNAMIC_DRAW);
    glNamedBufferData(r.ebo, batch.idxCount * sizeof(unsigned int), BUF(unsigned int, batch.indices), GL_DYNAMIC_DRAW);
}

void upload_commands(Render3D &r, const MeshBatch &batch)
{
    glNamedBufferData(r.commandBuffer, (batch.flatCount + batch.texCount) * sizeof(DrawCommand), nullptr,
                      GL_DYNAMIC_DRAW);
    glNamedBufferSubData(r.commandBuffer, 0, batch.flatCount * sizeof(DrawCommand), BUF(DrawCommand, batch.flatCmds));
    glNamedBufferSubData(r.commandBuffer, batch.flatCount * sizeof(DrawCommand), batch.texCount * sizeof(DrawCommand),
                         BUF(DrawCommand, batch.texCmds));
}

void upload_models(const Render3D &r, const MeshBatch &batch)
{
    glNamedBufferData(r.modelBuffer, batch.slotCount * sizeof(Mat4), BUF(Mat4, batch.models), GL_DYNAMIC_DRAW);
}

void upload_face_colors(const Render3D &r, const MeshBatch &batch)
{
    glNamedBufferData(r.faceColorBuffer, batch.primCount * sizeof(Color), BUF(Color, batch.primColors),
                      GL_DYNAMIC_DRAW);
    glNamedBufferData(r.faceOffsetBuffer, batch.slotCount * sizeof(int), BUF(int, batch.primOffsets), GL_DYNAMIC_DRAW);
}

void draw_flat(const Render3D &r, const MeshBatch &batch)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r.faceColorBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r.faceOffsetBuffer);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, r.commandBuffer);
    glMultiDrawElementsIndirect(r.primitive, GL_UNSIGNED_INT, (void *)0, batch.flatCount, 0);
}

void draw_textured(const Render3D &r, const MeshBatch &batch, const ModelTextureCache &modelTexCache)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, r.commandBuffer);
    for (int m = 0; m < modeltexture::count(modelTexCache); m++)
    {
        if (BUF(int, batch.texMatCount)[m] == 0)
        {
            continue;
        }
        glBindTextureUnit(0, modeltexture::get(modelTexCache, m));
        glMultiDrawElementsIndirect(r.primitive, GL_UNSIGNED_INT,
                                    (void *)((batch.flatCount + BUF(int, batch.texMatStart)[m]) * sizeof(DrawCommand)),
                                    BUF(int, batch.texMatCount)[m], 0);
    }
}

void free(Render3D &r)
{
    glDeleteVertexArrays(1, &r.vao);
    glDeleteBuffers(1, &r.vbo);
    glDeleteBuffers(1, &r.uvVbo);
    glDeleteBuffers(1, &r.ebo);
    glDeleteBuffers(1, &r.commandBuffer);
    glDeleteBuffers(1, &r.modelBuffer);
    glDeleteBuffers(1, &r.faceColorBuffer);
    glDeleteBuffers(1, &r.faceOffsetBuffer);
    r.vao = r.vbo = r.uvVbo = r.ebo = r.commandBuffer = r.modelBuffer = r.faceColorBuffer = r.faceOffsetBuffer = 0;
}

} // namespace render3d
