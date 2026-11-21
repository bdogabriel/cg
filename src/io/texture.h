#ifndef IO_TEXTURE_H
#define IO_TEXTURE_H

#include "renderer.h"

namespace texture
{

// decode an image file and upload it as an RGBA8 texture
render::TextureHandle load(const char *path);

} // namespace texture

#endif // IO_TEXTURE_H
