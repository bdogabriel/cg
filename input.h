#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

const int KEY_SCROLL_UP   = GLFW_KEY_LAST + 1;
const int KEY_SCROLL_DOWN = GLFW_KEY_LAST + 2;

struct Input
{
    bool keys[1024] = {};
    int  mods           = 0;
    double scrollDeltaY = 0;
    bool mouseLeftDown = false;
    bool mouseRightDown = false;
    double mouseX = 0, mouseY = 0;
    double mouseDeltaX = 0, mouseDeltaY = 0;

    void setup(GLFWwindow *window);
    void reset();
};

#endif // INPUT_H
