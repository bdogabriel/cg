#include "skybox.h"
#include "gl_util.h"
#include "stb_image.h"

#include <stdio.h>
#include <string.h>

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

bool load(Skybox &s, const char *path)
{
    int w = 0, h = 0, comp = 0;
    unsigned char *data = stbi_load(path, &w, &h, &comp, 4);
    if (!data)
    {
        printf("E: cannot load skybox %s: %s\n", path, stbi_failure_reason());
        return false;
    }

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (w > maxSize || h > maxSize)
    {
        printf("E: skybox %s (%dx%d) exceeds GL_MAX_TEXTURE_SIZE (%d)\n", path, w, h, (int)maxSize);
        stbi_image_free(data);
        return false;
    }

    if (s.texture != 0)
    {
        glDeleteTextures(1, &s.texture);
    }
    s.texture = glutil::make_texture(w, h, data, GL_RGBA8, GL_RGBA);
    glTextureParameteri(s.texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(s.texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);

    strncpy(s.path, path, sizeof(s.path) - 1);
    s.path[sizeof(s.path) - 1] = '\0';

    printf("loaded skybox %s (%dx%d)\n", path, w, h);
    return true;
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

void free(Skybox &s)
{
    if (s.texture != 0)
    {
        glDeleteTextures(1, &s.texture);
    }
    if (s.vao != 0)
    {
        glDeleteVertexArrays(1, &s.vao);
    }
    if (s.vbo != 0)
    {
        glDeleteBuffers(1, &s.vbo);
    }
    s.texture = 0;
    s.vao = 0;
    s.vbo = 0;
    s.path[0] = '\0';
}

} // namespace skybox
