#include "color.h"

namespace color
{
Color next_color(Color current)
{
    for (int i = 0; i < paletteSize; i++)
    {
        if (palette[i].r == current.r && palette[i].g == current.g && palette[i].b == current.b)
        {
            return palette[(i + 1) % paletteSize];
        }
    }
    return palette[0];
}
} // namespace color
