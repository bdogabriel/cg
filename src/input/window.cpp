#include "window.h"

namespace window
{

void create(Window &w, int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    w.handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(w.handle);
}

void destroy(Window &w)
{
    if (w.handle)
    {
        glfwDestroyWindow(w.handle);
        w.handle = nullptr;
    }
    glfwTerminate();
}

bool should_close(const Window &w)
{
    return glfwWindowShouldClose(w.handle);
}

void request_close(Window &w)
{
    if (w.handle)
    {
        glfwSetWindowShouldClose(w.handle, GLFW_TRUE);
    }
}

void poll_events()
{
    glfwPollEvents();
}

void swap_buffers(Window &w)
{
    glfwSwapBuffers(w.handle);
}

GLFWwindow *native(Window &w)
{
    return w.handle;
}

} // namespace window
