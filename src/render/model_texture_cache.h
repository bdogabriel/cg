#ifndef MODEL_TEXTURE_CACHE_H
#define MODEL_TEXTURE_CACHE_H

namespace modeltexture
{
constexpr int MAX_MODEL_TEXTURES = 200;
} // namespace modeltexture

struct ModelTextureCache
{
    unsigned int textures[modeltexture::MAX_MODEL_TEXTURES] = {};
    char paths[modeltexture::MAX_MODEL_TEXTURES][256] = {};
    int count = 0;
};

namespace modeltexture
{
int add(ModelTextureCache &modelTexCache, const char *path);
unsigned int get(const ModelTextureCache &modelTexCache, int index);
int count(const ModelTextureCache &modelTexCache);
void reset(ModelTextureCache &modelTexCache);
} // namespace modeltexture

#endif // MODEL_TEXTURE_CACHE_H
