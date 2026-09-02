#include "render1d.h"

namespace render1d
{
void init(Render1d &r)
{
    glCreateVertexArrays(1, &r.vao);
    glCreateBuffers(1, &r.posVbo);
    glCreateBuffers(1, &r.colVbo);

    glNamedBufferData(r.posVbo, line::MAX_VERTICES * sizeof(Vec4), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(r.colVbo, line::MAX_VERTICES * sizeof(Color), nullptr, GL_DYNAMIC_DRAW);

    glVertexArrayVertexBuffer(r.vao, 0, r.posVbo, 0, sizeof(Vec4));
    glVertexArrayVertexBuffer(r.vao, 1, r.colVbo, 0, sizeof(Color));

    glEnableVertexArrayAttrib(r.vao, 0);
    glVertexArrayAttribFormat(r.vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r.vao, 0, 0);

    glEnableVertexArrayAttrib(r.vao, 1);
    glVertexArrayAttribFormat(r.vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glVertexArrayAttribBinding(r.vao, 1, 1);
}

void draw(Render1d &r, const LineBatch &batch)
{
    if (batch.vtxCount == 0)
    {
        return;
    }
    glNamedBufferSubData(r.posVbo, 0, batch.vtxCount * sizeof(Vec4), batch.positions);
    glNamedBufferSubData(r.colVbo, 0, batch.vtxCount * sizeof(Color), batch.colors);
    glBindVertexArray(r.vao);
    glDrawArrays(GL_LINES, 0, batch.vtxCount);
}

void free(Render1d &r)
{
    glDeleteVertexArrays(1, &r.vao);
    glDeleteBuffers(1, &r.posVbo);
    glDeleteBuffers(1, &r.colVbo);
    r.vao = r.posVbo = r.colVbo = 0;
}
} // namespace render1d
