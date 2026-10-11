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

void upload_mesh(const Render3D &r, const MeshBatch &buf)
{
    glNamedBufferData(r.vbo, buf.vtxCount * sizeof(Vec4), buf.vertices, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.uvVbo, buf.vtxCount * sizeof(Vec2), buf.uvs, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.ebo, buf.idxCount * sizeof(unsigned int), buf.indices, GL_DYNAMIC_DRAW);
}

void upload_commands(Render3D &r, const MeshBatch &buf)
{
    glNamedBufferData(r.commandBuffer, (buf.flatCount + buf.texCount) * sizeof(DrawCommand), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(r.commandBuffer, 0, buf.flatCount * sizeof(DrawCommand), buf.flatCmds);
    glNamedBufferSubData(r.commandBuffer, buf.flatCount * sizeof(DrawCommand), buf.texCount * sizeof(DrawCommand),
                         buf.texCmds);
}

void upload_models(const Render3D &r, const MeshBatch &buf)
{
    glNamedBufferData(r.modelBuffer, buf.slotCount * sizeof(Mat4), buf.models, GL_DYNAMIC_DRAW);
}

void upload_face_colors(const Render3D &r, const MeshBatch &buf)
{
    glNamedBufferData(r.faceColorBuffer, buf.primCount * sizeof(Color), buf.primColors, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.faceOffsetBuffer, buf.slotCount * sizeof(int), buf.primOffsets, GL_DYNAMIC_DRAW);
}

void draw_flat(const Render3D &r, const MeshBatch &buf)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r.faceColorBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r.faceOffsetBuffer);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, r.commandBuffer);
    glMultiDrawElementsIndirect(r.primitive, GL_UNSIGNED_INT, (void *)0, buf.flatCount, 0);
}

void draw_textured(const Render3D &r, const MeshBatch &buf, const ModelTextureCache &modelTexCache)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, r.commandBuffer);
    for (int m = 0; m < modeltexture::count(modelTexCache); m++)
    {
        if (buf.texMatCount[m] == 0)
        {
            continue;
        }
        glBindTextureUnit(0, modeltexture::get(modelTexCache, m));
        glMultiDrawElementsIndirect(r.primitive, GL_UNSIGNED_INT,
                                    (void *)((buf.flatCount + buf.texMatStart[m]) * sizeof(DrawCommand)),
                                    buf.texMatCount[m], 0);
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
