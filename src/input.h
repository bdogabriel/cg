#ifndef INPUT_H
#define INPUT_H

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

struct Input
{
    KeyState keyStates[(int)Key::COUNT] = {};
    uint8_t mods = mods::NONE;
};

namespace input
{
void setup(Input &inp, GLFWwindow *window);
void reset(Input &inp);
} // namespace input

#endif // INPUT_H
