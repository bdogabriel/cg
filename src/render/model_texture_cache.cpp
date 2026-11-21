#include "model_texture_cache.h"
#include "texture.h"

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

    render::TextureHandle tex = texture::load(path);
    if (tex == 0)
    {
        return -1;
    }

    buffer::ensure(modelTexCache.textures, sizeof(render::TextureHandle), modelTexCache.count + 1);
    buffer::ensure(modelTexCache.paths, 256, modelTexCache.count + 1);

    int idx = modelTexCache.count++;
    BUF(render::TextureHandle, modelTexCache.textures)[idx] = tex;
    strncpy(BUF(TexturePath, modelTexCache.paths)[idx], path, 255);
    BUF(TexturePath, modelTexCache.paths)[idx][255] = '\0';

    return idx;
}

render::TextureHandle get(const ModelTextureCache &modelTexCache, int index)
{
    return BUF(render::TextureHandle, modelTexCache.textures)[index];
}

int count(const ModelTextureCache &modelTexCache)
{
    return modelTexCache.count;
}

void reset(ModelTextureCache &modelTexCache)
{
    for (int i = 0; i < modelTexCache.count; i++)
    {
        render::TextureHandle &tex = BUF(render::TextureHandle, modelTexCache.textures)[i];
        if (tex != 0)
        {
            render::destroy_texture(tex);
        }
        tex = 0;
        BUF(TexturePath, modelTexCache.paths)[i][0] = '\0';
    }
    modelTexCache.count = 0;
}

} // namespace modeltexture
