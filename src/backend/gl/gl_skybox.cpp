#include "gl_skybox.h"

#include <GL/glew.h>

namespace glskybox
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
    glCreateSamplers(1, &s.sampler);

    glVertexArrayVertexBuffer(s.vao, 0, s.vbo, 0, 3 * sizeof(float));
    glEnableVertexArrayAttrib(s.vao, 0);
    glVertexArrayAttribFormat(s.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(s.vao, 0, 0);

    glNamedBufferData(s.vbo, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

    glSamplerParameteri(s.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(s.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(s.sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(s.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void draw(const Skybox &s, unsigned int texture)
{
    glBindVertexArray(s.vao);
    glBindTextureUnit(0, texture);
    glBindSampler(0, s.sampler);
    glDrawArrays(GL_TRIANGLES, 0, CUBE_VERTEX_COUNT);
    glBindSampler(0, 0);
}

void free(Skybox &s)
{
    if (s.vao != 0)
    {
        glDeleteVertexArrays(1, &s.vao);
    }
    if (s.vbo != 0)
    {
        glDeleteBuffers(1, &s.vbo);
    }
    if (s.sampler != 0)
    {
        glDeleteSamplers(1, &s.sampler);
    }
    s.vao = 0;
    s.vbo = 0;
    s.sampler = 0;
}

} // namespace glskybox
