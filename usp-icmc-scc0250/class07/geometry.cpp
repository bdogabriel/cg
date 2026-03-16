#include "geometry.h"

void GeometryBuffer::upload(GLuint idxVertex)
{
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);
    glCreateVertexArrays(1, &vao);

    glNamedBufferData(vbo, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(ebo, i.size() * sizeof(unsigned int), i.data(), GL_DYNAMIC_DRAW);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 4 * sizeof(float));
    glVertexArrayElementBuffer(vao, ebo);

    glEnableVertexArrayAttrib(vao, idxVertex);
    glVertexArrayAttribFormat(vao, idxVertex, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, idxVertex, 0);
}

void GeometryBuffer::update() const
{
    glNamedBufferData(vbo, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(ebo, i.size() * sizeof(unsigned int), i.data(), GL_DYNAMIC_DRAW);
}

void GeometryBuffer::draw() const
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, i.size(), GL_UNSIGNED_INT, 0);
}

void GeometryBuffer::free()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    vao = vbo = ebo = 0;
}

GeometryBuffer geometry::cube()
{
    return GeometryBuffer{.v = std::vector(CUBE_V.begin(), CUBE_V.end()),
                          .i = std::vector(CUBE_I.begin(), CUBE_I.end())};
}
