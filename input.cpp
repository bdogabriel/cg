#include "input.h"

namespace input
{
static Input *gInput = nullptr;

static void scroll_callback(GLFWwindow *window, double dx, double dy)
{
    gInput->scrollDeltaY = dy;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    bool pressed = action == GLFW_PRESS;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        gInput->mouseLeftDown = pressed;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        gInput->mouseRightDown = pressed;
    }

    if (pressed)
    {
        glfwGetCursorPos(window, &gInput->mouseX, &gInput->mouseY);
    }
}

static void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    if (gInput->mouseLeftDown || gInput->mouseRightDown)
    {
        gInput->mouseDeltaX = x - gInput->mouseX;
        gInput->mouseDeltaY = y - gInput->mouseY;
        gInput->mouseX = x;
        gInput->mouseY = y;
    }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        gInput->keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        gInput->keys[key] = false;
    }
}

void setup_input(GLFWwindow *window, Input &input)
{
    gInput = &input;
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
}
} // namespace input
