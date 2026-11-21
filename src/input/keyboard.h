#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

struct Window;

enum class KeyState
{
    UP,
    JUST_PRESSED,
    DOWN,
    RELEASED
};

enum class Key : uint8_t
{
    NONE,
    ESC,
    T,
    R,
    S,
    A,
    F,
    W,
    X,
    Y,
    Z,
    N,
    P,
    E,
    M,
    D,
    O,
    C,
    SPACE,
    H,
    L,
    J,
    K,
    U,
    SEMICOLON,
    ENTER,
    BACKSPACE,
    COUNT
};

namespace mods
{
constexpr uint8_t NONE = 0;
constexpr uint8_t SHIFT = 1u << 0;
constexpr uint8_t CTRL = 1u << 1;
constexpr uint8_t ALT = 1u << 2;
constexpr uint8_t ALL = SHIFT | CTRL | ALT;
} // namespace mods

struct Keyboard
{
    KeyState keyStates[(int)Key::COUNT] = {};
    uint8_t mods = mods::NONE;
    char chars[64] = {};
    int charCount = 0;
};

namespace keyboard
{
void setup(Keyboard &kb, Window &window);
void reset(Keyboard &kb);
const char *key_name(Key k);
const char *mod_name(uint8_t mod);
} // namespace keyboard

#endif // KEYBOARD_H
