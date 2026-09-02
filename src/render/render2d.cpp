#include "render2d.h"

void render2d::init(Render2d &r, const Font &font, const uint8_t *atlas)
{
    glCreateVertexArrays(1, &r.vao);
    glCreateBuffers(1, &r.posVbo);
    glCreateBuffers(1, &r.uvVbo);
    glCreateBuffers(1, &r.colVbo);
    glCreateBuffers(1, &r.ebo);

    glNamedBufferData(r.posVbo, quad::MAX_VERTICES * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.uvVbo, quad::MAX_VERTICES * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.colVbo, quad::MAX_VERTICES * sizeof(Color), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.ebo, quad::MAX_INDICES * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    glVertexArrayVertexBuffer(r.vao, 0, r.posVbo, 0, 2 * sizeof(float));
    glVertexArrayVertexBuffer(r.vao, 1, r.uvVbo, 0, 2 * sizeof(float));
    glVertexArrayVertexBuffer(r.vao, 2, r.colVbo, 0, sizeof(Color));
    glVertexArrayElementBuffer(r.vao, r.ebo);

    glEnableVertexArrayAttrib(r.vao, 0);
    glVertexArrayAttribFormat(r.vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, 0, 0);

    glEnableVertexArrayAttrib(r.vao, 1);
    glVertexArrayAttribFormat(r.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, 1, 1);

    glEnableVertexArrayAttrib(r.vao, 2);
    glVertexArrayAttribFormat(r.vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glVertexArrayAttribBinding(r.vao, 2, 2);

    glCreateTextures(GL_TEXTURE_2D, 1, &r.atlas);
    glTextureParameteri(r.atlas, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(r.atlas, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(r.atlas, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(r.atlas, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(r.atlas, 1, GL_R8, font.atlasWidth, font.atlasHeight);
    glTextureSubImage2D(r.atlas, 0, 0, 0, font.atlasWidth, font.atlasHeight, GL_RED, GL_UNSIGNED_BYTE, atlas);
}

void render2d::draw(const Render2d &r, const QuadBatch &batch)
{
    if (batch.indexCount == 0)
    {
        return;
    }
    glNamedBufferSubData(r.posVbo, 0, batch.vertexCount * 2 * sizeof(float), batch.positions);
    glNamedBufferSubData(r.uvVbo, 0, batch.vertexCount * 2 * sizeof(float), batch.uvs);
    glNamedBufferSubData(r.colVbo, 0, batch.vertexCount * sizeof(Color), batch.colors);
    glNamedBufferSubData(r.ebo, 0, batch.indexCount * sizeof(unsigned int), batch.indices);

    glBindVertexArray(r.vao);
    glBindTextureUnit(0, r.atlas);
    glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, nullptr);
}

void render2d::free(Render2d &r)
{
    glDeleteVertexArrays(1, &r.vao);
    glDeleteBuffers(1, &r.posVbo);
    glDeleteBuffers(1, &r.uvVbo);
    glDeleteBuffers(1, &r.colVbo);
    glDeleteBuffers(1, &r.ebo);
    glDeleteTextures(1, &r.atlas);
    r = {};
}
