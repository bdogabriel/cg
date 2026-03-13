#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <filesystem>
#include <fstream>

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

float tX = 0.0f, tY = 0.0f, angle = 0.0f;

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

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(720, 720, "Class 04", NULL, NULL);

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

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);

    std::filesystem::path exePath = argv[0];

    std::string vCodeStr = str_from_file(exePath.parent_path() / "shader/vertex.glsl");
    std::string fCodeStr = str_from_file(exePath.parent_path() / "shader/fragment.glsl");

    const GLchar *vCode = vCodeStr.c_str();
    const GLchar *fCode = fCodeStr.c_str();

    glShaderSource(vertex, 1, &vCode, NULL);
    glShaderSource(fragment, 1, &fCode, NULL);

    GLint success;
    char infoLog[512];

    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        throw std::runtime_error("Vertex shader compilation failed");
    }

    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        throw std::runtime_error("Fragment shader compilation failed");
    }

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(fragment, 512, NULL, infoLog);
        throw std::runtime_error("Linking error");
    }

    glUseProgram(program);

    float vertices[] = {0.05f, -0.05f, 0.05f, 0.05f, -0.05f, -0.05f, -0.05f, 0.05f};

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    GLint loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    GLfloat R = 0.2f, G = 0.3f, B = 0.8f;

    GLint locColor = glGetUniformLocation(program, "color");

    glfwSetKeyCallback(window, key_callback);
    glfwShowWindow(window);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        float speed = 0.01f;

        if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_W] || keys[GLFW_KEY_K])
        {
            tY += speed;
        }
        if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_S] || keys[GLFW_KEY_J])
        {
            tY -= speed;
        }
        if (keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_A] || keys[GLFW_KEY_H])
        {
            tX -= speed;
        }
        if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_D] || keys[GLFW_KEY_L])
        {
            tX += speed;
        }

        angle -= 0.01;
        float c = std::cos(angle);
        float s = std::sin(angle);
        float transform[] = {c, -s, 0.0f, tX, s, c, 0.0f, tY, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        loc = glGetUniformLocation(program, "mat_transformation");
        glUniformMatrix4fv(loc, 1, GL_TRUE, transform);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUniform4f(locColor, R, G, B, 1.0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
