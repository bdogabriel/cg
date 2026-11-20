#include "skybox.h"

#include <GL/glew.h>

namespace skybox
{

namespace
{

// unit cube; 36 vertices (6 faces x 2 triangles x 3 verts); 3 floats each
// culling is not enabled, so faces are visible from inside regardless of winding
const float CUBE_VERTICES[] = {
    ////// +X
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    1.0f,
    ////// -X
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    ////// +Y
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    ////// -Y
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    ////// +Z
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    ////// -Z
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
};

constexpr int CUBE_VERTEX_COUNT = 36;

} // namespace

void init(Skybox &s)
{
    glCreateVertexArrays(1, &s.vao);
    glCreateBuffers(1, &s.vbo);

    glVertexArrayVertexBuffer(s.vao, 0, s.vbo, 0, 3 * sizeof(float));
    glEnableVertexArrayAttrib(s.vao, 0);
    glVertexArrayAttribFormat(s.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(s.vao, 0, 0);

    glNamedBufferData(s.vbo, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);
}

void set_texture(Skybox &s, unsigned int texture)
{
    if (s.texture != 0)
    {
        glDeleteTextures(1, &s.texture);
    }
    s.texture = texture;
    if (texture != 0)
    {
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void draw(const Skybox &s)
{
    if (s.texture == 0)
    {
        return;
    }
    glBindVertexArray(s.vao);
    glBindTextureUnit(0, s.texture);
    glDrawArrays(GL_TRIANGLES, 0, CUBE_VERTEX_COUNT);
}

void reset(Skybox &s)
{
    set_texture(s, 0);
}

void free(Skybox &s)
{
    reset(s);
    if (s.vao != 0)
    {
        glDeleteVertexArrays(1, &s.vao);
    }
    if (s.vbo != 0)
    {
        glDeleteBuffers(1, &s.vbo);
    }
    s.vao = 0;
    s.vbo = 0;
}

} // namespace skybox
