#include "entity.h"
#include "geometry.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>

// TODO: refactor trs (use quaternions)
// TODO: extract input handling

constexpr GLint LOC_TRANSFORM = 0;
constexpr GLint LOC_COLOR = 1;
constexpr GLuint IDX_VERTEX = 0;

std::array<bool, 1024> keys;
bool mouseLeftDown = false;
bool mouseRightDown = false;
double mouseX = 0, mouseY = 0;
double mouseDeltaX = 0, mouseDeltaY = 0;
double scrollDeltaY = 0;

enum EditMode
{
    TRANSLATE,
    ROTATE,
    SCALE
};

void scroll_callback(GLFWwindow *window, double dx, double dy)
{
    scrollDeltaY = dy;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    bool pressed = action == GLFW_PRESS;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        mouseLeftDown = pressed;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        mouseRightDown = pressed;
    }

    if (pressed)
    {
        glfwGetCursorPos(window, &mouseX, &mouseY);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    if (mouseLeftDown || mouseRightDown)
    {
        mouseDeltaX = x - mouseX;
        mouseDeltaY = y - mouseY;
        mouseX = x;
        mouseY = y;
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = false;
    }
}

TRS handle_input(EditMode &mode)
{
    float keySensitivity = 0.05f;
    float mouseSensitivity = 0.005f;
    float scrollSensitivity = 0.1f;

    TRS t;

    if (mouseRightDown)
    {
        t.tx += mouseDeltaX * mouseSensitivity;
        t.ty -= mouseDeltaY * mouseSensitivity;
        mouseDeltaX = mouseDeltaY = 0;
    }

    if (scrollDeltaY)
    {
        float scale = 1.0f + scrollDeltaY * scrollSensitivity;
        t.sx = t.sy = t.sz = scale;
        scrollDeltaY = 0;
    }

    if (keys[GLFW_KEY_T])
    {
        mode = TRANSLATE;
    }
    if (keys[GLFW_KEY_R])
    {
        mode = ROTATE;
    }
    if (keys[GLFW_KEY_S])
    {
        mode = SCALE;
    }

    float x = (keys[GLFW_KEY_RIGHT] - keys[GLFW_KEY_LEFT]) * keySensitivity;
    float y = (keys[GLFW_KEY_UP] - keys[GLFW_KEY_DOWN]) * keySensitivity;
    float z = (keys[GLFW_KEY_PAGE_UP] - keys[GLFW_KEY_PAGE_DOWN]) * keySensitivity;

    switch (mode)
    {
    case TRANSLATE:
        t.tx += x;
        t.ty += y;
        t.tz += z;
        break;
    case ROTATE:
        t.ry += x;
        t.rx -= y;
        t.rz += z;
        break;
    case SCALE:
        t.sx += x;
        t.sy += y;
        t.sz += z;
        break;
    }

    t.rx = std::fmod(t.rx, 2 * M_PI);
    t.ry = std::fmod(t.ry, 2 * M_PI);
    t.rz = std::fmod(t.rz, 2 * M_PI);

    return t;
}

std::string str_from_file(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error(std::string("Failed to open file: ") + path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

void load_shader(GLuint program, const std::string &code, GLenum type)
{
    GLuint shader = glCreateShader(type);
    const char *codeCStr = code.c_str();

    glShaderSource(shader, 1, &codeCStr, NULL);
    glCompileShader(shader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }

    glAttachShader(program, shader);
}

GLFWwindow *setup_window()
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(800, 800, "Class 07", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwShowWindow(window);

    return window;
}

GLuint setup_program(const std::filesystem::path &exePath)
{
    GLuint program = glCreateProgram();

    std::string vCode = str_from_file(exePath.parent_path() / "shader/vertex.glsl");
    std::string fCode = str_from_file(exePath.parent_path() / "shader/fragment.glsl");

    load_shader(program, vCode, GL_VERTEX_SHADER);
    load_shader(program, fCode, GL_FRAGMENT_SHADER);

    glLinkProgram(program);
    glUseProgram(program);

    return program;
}

void setup_graphics()
{
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = setup_window();
    std::filesystem::path exePath = argv[0];
    GLuint program = setup_program(exePath);
    setup_graphics();

    Entity cube = {.geometry = geometry::cube(),
                   .transform = {.sx = 0.5f, .sy = 0.5f, .sz = 0.5f},
                   .color = {0.2f, 0.3f, 0.8f, 1}};
    Entity axisX = {.geometry = geometry::axis_x(), .color = {1, 0, 0, 1}, .primitive = GL_LINES};
    Entity axisY = {.geometry = geometry::axis_y(), .color = {0, 1, 0, 1}, .primitive = GL_LINES};
    Entity axisZ = {.geometry = geometry::axis_z(), .color = {0, 0, 1, 1}, .primitive = GL_LINES};

    std::array<Entity *, 4> scene = {&cube, &axisX, &axisY, &axisZ};

    for (Entity *e : scene)
    {
        e->geometry.upload(IDX_VERTEX);
    }

    EditMode mode = ROTATE;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        TRS t = handle_input(mode);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (Entity *e : scene)
        {
            e->transform += t;
            e->build_matrix();
            e->draw(LOC_TRANSFORM, LOC_COLOR);
        }

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
