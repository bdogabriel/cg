#include "input.h"

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
    glfwSetKeyCallback(window, key_callback);
}

void Input::reset()
{
    for (auto &k : keys)
    {
        if (k == KeyState::Released)
        {
            k = KeyState::Up;
        }
    }
}
