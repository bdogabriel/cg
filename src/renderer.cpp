#include "renderer.h"

namespace render
{

void init(Renderer &r, GLuint idxVertex)
{
    glCreateVertexArrays(1, &r.vao);

    glCreateBuffers(1, &r.vbo);
    glCreateBuffers(1, &r.ebo);
    glCreateBuffers(1, &r.commandBuffer);
    glCreateBuffers(1, &r.modelBuffer);
    glCreateBuffers(1, &r.faceColorBuffer);
    glCreateBuffers(1, &r.faceOffsetBuffer);

    glVertexArrayVertexBuffer(r.vao, 0, r.vbo, 0, sizeof(Vec4));
    glVertexArrayElementBuffer(r.vao, r.ebo);

    glEnableVertexArrayAttrib(r.vao, idxVertex);
    glVertexArrayAttribFormat(r.vao, idxVertex, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, idxVertex, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r.faceColorBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r.faceOffsetBuffer);
}

void upload_mesh(const Renderer &r, const Buffer &buf)
{
    glNamedBufferData(r.vbo, buf.vtxCount * sizeof(Vec4), buf.vertices, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.ebo, buf.idxCount * sizeof(unsigned int), buf.indices, GL_DYNAMIC_DRAW);
}

void upload_commands(Renderer &r, const Buffer &buf)
{
    glNamedBufferData(r.commandBuffer, buf.slotCount * sizeof(DrawCommand), buf.drawCmds, GL_DYNAMIC_DRAW);
    r.slotCount = buf.slotCount;
}

void upload_models(const Renderer &r, const Buffer &buf)
{
    glNamedBufferData(r.modelBuffer, buf.slotCount * sizeof(Mat4), buf.models, GL_DYNAMIC_DRAW);
}

void upload_face_colors(const Renderer &r, const Buffer &buf, int faceVtxCount)
{
    int totalFaces = buf.idxCount / faceVtxCount;
    glNamedBufferData(r.faceColorBuffer, totalFaces * sizeof(Color), buf.faceColors, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.faceOffsetBuffer, buf.slotCount * sizeof(int), buf.faceOffsets, GL_DYNAMIC_DRAW);
}

void draw(const Renderer &r)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.modelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r.faceColorBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r.faceOffsetBuffer);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, r.commandBuffer);
    glMultiDrawElementsIndirect(r.primitive, GL_UNSIGNED_INT, 0, r.slotCount, 0);
}

void free(Renderer &r)
{
    glDeleteVertexArrays(1, &r.vao);
    glDeleteBuffers(1, &r.vbo);
    glDeleteBuffers(1, &r.ebo);
    glDeleteBuffers(1, &r.commandBuffer);
    glDeleteBuffers(1, &r.modelBuffer);
    glDeleteBuffers(1, &r.faceColorBuffer);
    glDeleteBuffers(1, &r.faceOffsetBuffer);
    r.vao = r.vbo = r.ebo = r.commandBuffer = r.modelBuffer = r.faceColorBuffer = r.faceOffsetBuffer = 0;
    r.slotCount = 0;
}

} // namespace render
