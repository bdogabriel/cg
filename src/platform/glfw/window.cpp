#include "window.h"

#include "glfw_internal.h"

#include <GLFW/glfw3.h>

#include <stdio.h>

namespace window
{

namespace
{

void error_callback(int code, const char *description)
{
    fprintf(stderr, "E: GLFW error %d: %s\n", code, description);
}

void framebuffer_size_callback(GLFWwindow *handle, int width, int height)
{
    (void)handle;
    Window *w = glfwplat::state().window;
    if (w)
    {
        w->fbWidth = width;
        w->fbHeight = height;
    }
}

GLFWwindow *handle_of(const Window &w)
{
    return static_cast<GLFWwindow *>(w.native);
}

} // namespace

bool create(Window &w, const WindowConfig &config)
{
    w = Window{};

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, config.createGraphicsContext ? GLFW_OPENGL_API : GLFW_NO_API);

    GLFWwindow *handle = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
    if (!handle)
    {
        glfwTerminate();
        return false;
    }

    w.native = handle;
    w.width = config.width;
    w.height = config.height;
    glfwGetFramebufferSize(handle, &w.fbWidth, &w.fbHeight);
    glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);

    if (config.createGraphicsContext)
    {
        glfwMakeContextCurrent(handle);
    }

    glfwplat::state().window = &w;
    return true;
}

void destroy(Window &w)
{
    if (w.native)
    {
        glfwDestroyWindow(handle_of(w));
    }
    glfwplat::state().window = nullptr;
    glfwplat::state().keyboard = nullptr;
    w = Window{};
    glfwTerminate();
}

void show(Window &w)
{
    if (w.native)
    {
        glfwShowWindow(handle_of(w));
    }
}

bool should_close(const Window &w)
{
    return w.native ? glfwWindowShouldClose(handle_of(w)) : true;
}

void request_close(Window &w)
{
    if (w.native)
    {
        glfwSetWindowShouldClose(handle_of(w), GLFW_TRUE);
    }
}

void poll_events()
{
    glfwPollEvents();
}

void swap_buffers(Window &w)
{
    if (w.native)
    {
        glfwSwapBuffers(handle_of(w));
    }
}

void *native(Window &w)
{
    return w.native;
}

} // namespace window
