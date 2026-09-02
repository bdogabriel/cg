#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <GLFW/glfw3.h>
#include <stdint.h>

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
constexpr uint8_t SHIFT = GLFW_MOD_SHIFT;
constexpr uint8_t CTRL = GLFW_MOD_CONTROL;
constexpr uint8_t ALT = GLFW_MOD_ALT;
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
void setup(Keyboard &kb, GLFWwindow *window);
void reset(Keyboard &kb);
} // namespace keyboard

#endif // KEYBOARD_H
