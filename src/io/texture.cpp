#include "texture.h"

#include "image.h"

namespace texture
{

render::TextureHandle load(const char *path)
{
    Image img;
    if (!image::load(path, img))
    {
        return 0;
    }

    render::TextureDesc desc;
    desc.width = img.width;
    desc.height = img.height;
    desc.format = render::TextureFormat::RGBA8;
    desc.pixels = img.pixels;

    render::TextureHandle handle = render::create_texture(desc);
    image::free(img);
    return handle;
}

} // namespace texture
