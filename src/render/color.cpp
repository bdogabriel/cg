#include "color.h"

namespace color
{
Color from_float(float r, float g, float b, float a)
{
    auto to_byte = [](float v) -> uint8_t {
        int iv = static_cast<int>(v * 255.0f + 0.5f);
        if (iv < 0)
            iv = 0;
        if (iv > 255)
            iv = 255;
        return static_cast<uint8_t>(iv);
    };
    return {to_byte(r), to_byte(g), to_byte(b), to_byte(a)};
}

bool from_hex(const char *hex, Color &out)
{
    const char *s = hex;
    if (*s == '#')
        s++;

    int len = 0;
    while (s[len] != '\0')
        len++;

    if (len != 6 && len != 8)
        return false;

    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };

    int r0 = nibble(s[0]), r1 = nibble(s[1]);
    int g0 = nibble(s[2]), g1 = nibble(s[3]);
    int b0 = nibble(s[4]), b1 = nibble(s[5]);
    if (r0 < 0 || r1 < 0 || g0 < 0 || g1 < 0 || b0 < 0 || b1 < 0)
        return false;

    out.r = (uint8_t)(r0 * 16 + r1);
    out.g = (uint8_t)(g0 * 16 + g1);
    out.b = (uint8_t)(b0 * 16 + b1);

    if (len == 8)
    {
        int a0 = nibble(s[6]), a1 = nibble(s[7]);
        if (a0 < 0 || a1 < 0)
            return false;
        out.a = (uint8_t)(a0 * 16 + a1);
    }
    else
    {
        out.a = 255;
    }
    return true;
}
} // namespace color
