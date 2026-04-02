#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

struct Input
{
    bool keys[1024] = {};
    float keySensitivity = 0.05f;
    float mouseSensitivity = 0.005f;
    float scrollSensitivity = 0.1f;
    double scrollDeltaY = 0;
    bool mouseLeftDown = false;
    bool mouseRightDown = false;
    double mouseX = 0, mouseY = 0;
    double mouseDeltaX = 0, mouseDeltaY = 0;
};

namespace input
{
void setup_input(GLFWwindow *window, Input &input);
} // namespace input

#endif // INPUT_H
