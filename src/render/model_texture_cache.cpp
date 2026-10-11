#include "model_texture_cache.h"
#include "gl_util.h"
#include "stb_image.h"

#include <stdio.h>
#include <string.h>

namespace modeltexture
{

static int find_by_path(const ModelTextureCache &modelTexCache, const char *path)
{
    for (int i = 0; i < modelTexCache.count; i++)
    {
        if (strcmp(modelTexCache.paths[i], path) == 0)
        {
            return i;
        }
    }
    return -1;
}

int add(ModelTextureCache &modelTexCache, const char *path)
{
    int existing = find_by_path(modelTexCache, path);
    if (existing >= 0)
    {
        return existing;
    }
    if (modelTexCache.count >= MAX_MODEL_TEXTURES)
    {
        printf("E: too many model textures (max %d)\n", MAX_MODEL_TEXTURES);
        return -1;
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

    int idx = modelTexCache.count++;
    modelTexCache.textures[idx] = tex;
    strncpy(modelTexCache.paths[idx], path, sizeof(modelTexCache.paths[idx]) - 1);
    modelTexCache.paths[idx][sizeof(modelTexCache.paths[idx]) - 1] = '\0';

    return idx;
}

unsigned int get(const ModelTextureCache &modelTexCache, int index)
{
    return modelTexCache.textures[index];
}

int count(const ModelTextureCache &modelTexCache)
{
    return modelTexCache.count;
}

void reset(ModelTextureCache &modelTexCache)
{
    for (int i = 0; i < modelTexCache.count; i++)
    {
        if (modelTexCache.textures[i] != 0)
        {
            glDeleteTextures(1, &modelTexCache.textures[i]);
        }
        modelTexCache.textures[i] = 0;
        modelTexCache.paths[i][0] = '\0';
    }
    modelTexCache.count = 0;
}

} // namespace modeltexture
