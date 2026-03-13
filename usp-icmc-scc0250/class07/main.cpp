#include "mat4.h"
#include "trs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>

TRS trs;

bool keys[1024] = {false};

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

void handle_input()
{
    float speed = 0.01;

    if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_W] || keys[GLFW_KEY_K])
    {
        trs.ty += speed;
    }
    if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_S] || keys[GLFW_KEY_J])
    {
        trs.ty -= speed;
    }
    if (keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_A] || keys[GLFW_KEY_H])
    {
        trs.tx -= speed;
    }
    if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_D] || keys[GLFW_KEY_L])
    {
        trs.tx += speed;
    }
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

void load_shader(GLuint program, std::string code, GLenum type)
{
    GLuint shader = glCreateShader(type);
    char *codeCStr = (char *)code.c_str();

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

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(800, 800, "Class 07", NULL, NULL);

    if (!window)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    if (!glewInit())
    {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    GLuint program = glCreateProgram();

    std::filesystem::path exePath = argv[0];

    std::string vCode = str_from_file(exePath.parent_path() / "shader/vertex.glsl");
    std::string fCode = str_from_file(exePath.parent_path() / "shader/fragment.glsl");

    load_shader(program, vCode, GL_VERTEX_SHADER);
    load_shader(program, fCode, GL_FRAGMENT_SHADER);

    GLint success;
    char infoLog[512];
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw std::runtime_error("Linking error");
    }

    glUseProgram(program);

    float squareSideLength = 0.5;
    float squareVertices[16] = {1, -1, 0, 0, 1, 1, 0, 0, -1, -1, 0, 0, -1, 1, 0, 0};

    for (int i = 0; i < 16; i++)
    {
        squareVertices[i] *= squareSideLength;
    }

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_DYNAMIC_DRAW);

    GLint loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    GLfloat R = 0.2, G = 0.3, B = 0.8;

    GLint locColor = glGetUniformLocation(program, "color");

    glfwSetKeyCallback(window, key_callback);
    glfwShowWindow(window);
    // glClearColor(1, 1, 1, 1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Mat4 trsMat;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        handle_input();

        trs.rx -= 0.01;
        trs.ry -= 0.01;
        trs.rz -= 0.01;

        trs.sx = 1.5;
        trs.sy = 0.5;
        trs.sz = 0.5;

        build_trs(trsMat, trs);

        identity(trsMat);
        translate(trsMat, trs);
        rotate(trsMat, trs);
        scale(trsMat, trs);

        loc = glGetUniformLocation(program, "mat_transform");
        glUniformMatrix4fv(loc, 1, GL_FALSE, trsMat.data());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUniform4f(locColor, R, G, B, 1.0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
