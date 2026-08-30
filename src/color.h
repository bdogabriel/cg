#ifndef COLOR_H
#define COLOR_H

// TODO: proper color support

struct Color
{
    float r = 0.5, g = 0.5, b = 0.5, a = 0.5;
};

namespace color
{
constexpr Color palette[] = {
    {1.0f, 0.2f, 0.2f, 1.0f}, // red
    {0.2f, 1.0f, 0.2f, 1.0f}, // green
    {0.2f, 0.6f, 1.0f, 1.0f}, // blue
    {1.0f, 1.0f, 0.2f, 1.0f}, // yellow
    {1.0f, 0.4f, 0.0f, 1.0f}, // orange
    {0.8f, 0.2f, 1.0f, 1.0f}, // purple
    {0.2f, 1.0f, 1.0f, 1.0f}, // cyan
    {1.0f, 0.5f, 0.7f, 1.0f}, // pink
};
constexpr int paletteSize = sizeof(palette) / sizeof(palette[0]);

Color next_color(Color current);
} // namespace color

#endif // COLOR_H
