#ifndef GLFW_INTERNAL_H
#define GLFW_INTERNAL_H

struct Window;
struct Keyboard;

namespace glfwplat
{

struct State
{
    Window *window = nullptr;
    Keyboard *keyboard = nullptr;
};

inline State &state()
{
    static State s;
    return s;
}

} // namespace glfwplat

#endif // GLFW_INTERNAL_H
