#include "input.h"

static void scroll_callback(GLFWwindow *window, double dx, double dy)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    inp->scrollDeltaY += dy;
    if (dy > 0)
    {
        inp->keys[KEY_SCROLL_UP] = KeyState::Down;
    }
    if (dy < 0)
    {
        inp->keys[KEY_SCROLL_DOWN] = KeyState::Down;
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    bool pressed = action == GLFW_PRESS;

    inp->keys[button] = pressed ? KeyState::Down : KeyState::Released;

    if (pressed)
    {
        glfwGetCursorPos(window, &inp->mouseX, &inp->mouseY);
    }
}

static void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    if (inp->keys[GLFW_MOUSE_BUTTON_LEFT] == KeyState::Down || inp->keys[GLFW_MOUSE_BUTTON_RIGHT] == KeyState::Down)
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
    {
        inp->keys[key] = KeyState::Down;
    }
    else if (action == GLFW_RELEASE)
    {
        inp->keys[key] = KeyState::Released;
    }

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
    keys[KEY_SCROLL_UP] = keys[KEY_SCROLL_DOWN] = KeyState::Up;
    for (auto &k : keys)
    {
        if (k == KeyState::Released)
        {
            k = KeyState::Up;
        }
    }
}
