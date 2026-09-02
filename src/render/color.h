#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

struct Color
{
    uint8_t r = 128, g = 128, b = 128, a = 128;
};

namespace color
{
constexpr Color palette[] = {
    {255, 51, 51, 255},   // red
    {51, 255, 51, 255},   // green
    {51, 153, 255, 255},  // blue
    {255, 255, 51, 255},  // yellow
    {255, 102, 0, 255},   // orange
    {204, 51, 255, 255},  // purple
    {51, 255, 255, 255},  // cyan
    {255, 127, 178, 255}, // pink
};
constexpr int paletteSize = sizeof(palette) / sizeof(palette[0]);

Color from_float(float r, float g, float b, float a = 1.0f);
bool from_hex(const char *hex, Color &out);
} // namespace color

#endif // COLOR_H
