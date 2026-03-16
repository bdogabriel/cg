#include "entity.h"
#include "geometry.h"
#include "trs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <filesystem>
#include <fstream>

std::array<bool, 1024> keys;

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

TRS handle_input()
{
    float speed = 0.01f;

    TRS t;

    t.rx -= speed;
    t.ry -= speed;
    t.rz -= speed;

    if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_W] || keys[GLFW_KEY_K])
    {
        t.ty += speed;
    }
    if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_S] || keys[GLFW_KEY_J])
    {
        t.ty -= speed;
    }
    if (keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_A] || keys[GLFW_KEY_H])
    {
        t.tx -= speed;
    }
    if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_D] || keys[GLFW_KEY_L])
    {
        t.tx += speed;
    }

    return t;
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

    cube.geometry.upload(0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        TRS t = handle_input();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.transform += t;
        cube.build();
        cube.draw(0, 1);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
