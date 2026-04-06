#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

enum class KeyState
{
    Up,
    JustPressed,
    Down,
    Released
};

struct Input
{
    KeyState keys[GLFW_KEY_LAST + 1] = {};
    int mods = 0;

    void setup(GLFWwindow *window);
    void reset();
};

#endif // INPUT_H
