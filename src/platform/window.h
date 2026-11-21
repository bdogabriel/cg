#ifndef WINDOW_H
#define WINDOW_H

struct Window
{
    void *native = nullptr;
    int width = 0;
    int height = 0;
    int fbWidth = 0;
    int fbHeight = 0;
};

struct WindowConfig
{
    int width = 1000;
    int height = 1000;
    const char *title = "Editor";
    bool visible = false;
    bool createGraphicsContext = true;
};

namespace window
{
bool create(Window &w, const WindowConfig &config);
void destroy(Window &w);
void show(Window &w);
bool should_close(const Window &w);
void request_close(Window &w);
void poll_events();
void swap_buffers(Window &w); // GL presentation
void *native(Window &w);
} // namespace window

#endif // WINDOW_H
