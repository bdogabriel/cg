#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

struct Window
{
    GLFWwindow *handle = nullptr;
};

namespace window
{
void create(Window &w, int width, int height, const char *title);
void destroy(Window &w);
bool should_close(const Window &w);
void request_close(Window &w);
void poll_events();
void swap_buffers(Window &w);
GLFWwindow *native(Window &w);
} // namespace window

#endif
