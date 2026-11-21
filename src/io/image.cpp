#include "image.h"

#include "stb_image.h"

#include <stdio.h>

namespace image
{

bool load(const char *path, Image &out)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channels, 4);
    if (!pixels)
    {
        printf("E: cannot load image %s: %s\n", path, stbi_failure_reason());
        return false;
    }

    out.width = width;
    out.height = height;
    out.pixels = pixels;
    return true;
}

void free(Image &img)
{
    if (img.pixels)
    {
        stbi_image_free(img.pixels);
        img.pixels = nullptr;
    }
    img.width = 0;
    img.height = 0;
}

} // namespace image
