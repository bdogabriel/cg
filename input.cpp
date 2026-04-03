#include "input.h"

static void scroll_callback(GLFWwindow *window, double dx, double dy)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    inp->scrollDeltaY += dy;
    inp->keys[KEY_SCROLL_UP] = dy > 0;
    inp->keys[KEY_SCROLL_DOWN] = dy < 0;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    bool pressed = action == GLFW_PRESS;

    inp->keys[button] = pressed;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
        inp->mouseLeftDown = pressed;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        inp->mouseRightDown = pressed;

    if (pressed)
        glfwGetCursorPos(window, &inp->mouseX, &inp->mouseY);
}

static void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    if (inp->mouseLeftDown || inp->mouseRightDown)
    {
        inp->mouseDeltaX += x - inp->mouseX;
        inp->mouseDeltaY += y - inp->mouseY;
        inp->mouseX = x;
        inp->mouseY = y;
    }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS)
        inp->keys[key] = true;
    else if (action == GLFW_RELEASE)
        inp->keys[key] = false;

    inp->mods = mods;
}

void Input::setup(GLFWwindow *window)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
}

void Input::reset()
{
    mouseDeltaX = mouseDeltaY = 0;
    scrollDeltaY = 0;
    keys[KEY_SCROLL_UP] = keys[KEY_SCROLL_DOWN] = false;
}
