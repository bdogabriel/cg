#ifndef IO_IMAGE_H
#define IO_IMAGE_H

#include <stdint.h>

// decoded RGBA8 image data
struct Image
{
    int width = 0;
    int height = 0;
    uint8_t *pixels = nullptr;
};

namespace image
{
bool load(const char *path, Image &out);
void free(Image &img);
} // namespace image

#endif // IO_IMAGE_H
