#ifndef MODEL_TEXTURE_CACHE_H
#define MODEL_TEXTURE_CACHE_H

#include "buffer.h"
#include "renderer.h"

typedef char TexturePath[256];

struct ModelTextureCache
{
    Buffer textures; // render::TextureHandle
    Buffer paths;
    int count = 0;
};

namespace modeltexture
{
int add(ModelTextureCache &modelTexCache, const char *path);
render::TextureHandle get(const ModelTextureCache &modelTexCache, int index);
int count(const ModelTextureCache &modelTexCache);
void reset(ModelTextureCache &modelTexCache);
} // namespace modeltexture

#endif // MODEL_TEXTURE_CACHE_H
