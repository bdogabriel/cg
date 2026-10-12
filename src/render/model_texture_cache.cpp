#include "model_texture_cache.h"
#include "gl_util.h"
#include "stb_image.h"

#include <stdio.h>
#include <string.h>

namespace modeltexture
{

namespace
{

int find_by_path(const ModelTextureCache &modelTexCache, const char *path)
{
    for (int i = 0; i < modelTexCache.count; i++)
    {
        if (strcmp(BUF(TexturePath, modelTexCache.paths)[i], path) == 0)
        {
            return i;
        }
    }
    return -1;
}

} // namespace

int add(ModelTextureCache &modelTexCache, const char *path)
{
    int existing = find_by_path(modelTexCache, path);
    if (existing >= 0)
    {
        return existing;
    }

    int w = 0, h = 0, comp = 0;
    unsigned char *data = stbi_load(path, &w, &h, &comp, 4);
    if (!data)
    {
        printf("E: cannot load texture %s: %s\n", path, stbi_failure_reason());
        return -1;
    }

    GLuint tex = glutil::make_texture(w, h, data, GL_RGBA8, GL_RGBA);
    stbi_image_free(data);

    buffer::ensure(modelTexCache.textures, sizeof(unsigned int), modelTexCache.count + 1);
    buffer::ensure(modelTexCache.paths, 256, modelTexCache.count + 1);

    int idx = modelTexCache.count++;
    BUF(unsigned int, modelTexCache.textures)[idx] = tex;
    strncpy(BUF(TexturePath, modelTexCache.paths)[idx], path, 255);
    BUF(TexturePath, modelTexCache.paths)[idx][255] = '\0';

    return idx;
}

unsigned int get(const ModelTextureCache &modelTexCache, int index)
{
    return BUF(unsigned int, modelTexCache.textures)[index];
}

int count(const ModelTextureCache &modelTexCache)
{
    return modelTexCache.count;
}

void reset(ModelTextureCache &modelTexCache)
{
    for (int i = 0; i < modelTexCache.count; i++)
    {
        if (BUF(unsigned int, modelTexCache.textures)[i] != 0)
        {
            glDeleteTextures(1, &BUF(unsigned int, modelTexCache.textures)[i]);
        }
        BUF(unsigned int, modelTexCache.textures)[i] = 0;
        BUF(TexturePath, modelTexCache.paths)[i][0] = '\0';
    }
    modelTexCache.count = 0;
}

} // namespace modeltexture
