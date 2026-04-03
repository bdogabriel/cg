#include "buffer.h"
#include "editor.h"
#include "input.h"
#include "mat4.h"
#include "trs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>

// TODO: track ball with quaternions

const GLuint IDX_VERTEX = 0;

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

    DrawBuffer triangles;
    CubeGeometry cubeGeo;
    TRS cubeTRS;
    cubeTRS.sx = cubeTRS.sy = cubeTRS.sz = 0.5f;
    Ref cubeRef = triangles.add(cubeGeo.vertices, 8, cubeGeo.indices, 36, trs::compose(cubeTRS), {0.2f, 0.3f, 0.8f, 1});
    triangles.init(IDX_VERTEX);
    triangles.update();

    DrawBuffer lines;
    lines.primitive = GL_LINES;
    Mat4 axisMat = mat4::IDENTITY;
    AxisXGeometry axGeo;
    AxisYGeometry ayGeo;
    AxisZGeometry azGeo;
    Ref axRef = lines.add(axGeo.vertices, 2, axGeo.indices, 2, axisMat, {1, 0, 0, 1});
    Ref ayRef = lines.add(ayGeo.vertices, 2, ayGeo.indices, 2, axisMat, {0, 1, 0, 1});
    Ref azRef = lines.add(azGeo.vertices, 2, azGeo.indices, 2, axisMat, {0, 0, 1, 1});
    lines.init(IDX_VERTEX);
    lines.update();

    Input input;
    input.setup(window);

    EditorState state;
    state.selected = &cubeTRS;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        editor::process_input(input, state);
        input.reset();

        Mat4 cubeMat = trs::compose(cubeTRS);
        axisMat = cubeMat;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        triangles.transforms[cubeRef] = cubeMat;
        triangles.update_transforms();
        triangles.draw();

        lines.transforms[axRef] = axisMat;
        lines.transforms[ayRef] = axisMat;
        lines.transforms[azRef] = axisMat;
        lines.update_transforms();
        lines.draw();

        glfwSwapBuffers(window);
    }

    triangles.free();
    lines.free();
    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
