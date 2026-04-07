#include "buffer.h"
#include "editor.h"
#include "input.h"
#include "mat4.h"
#include "scene.h"
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
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Handmade", NULL, NULL);
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = setup_window();
    std::filesystem::path exePath = argv[0];
    GLuint program = setup_program(exePath);
    setup_graphics();

    DrawBuffer triangles;
    triangles.init(IDX_VERTEX);
    triangles.update();

    DrawBuffer highlights;
    highlights.primitive = GL_LINES;
    highlights.init(IDX_VERTEX);

    DrawBuffer lines;
    lines.primitive = GL_LINES;
    Mat4 axisMat = mat4::IDENTITY;
    Ref axRef = lines.add(geo::axisX, TRS{});
    Ref ayRef = lines.add(geo::axisY, TRS{});
    Ref azRef = lines.add(geo::axisZ, TRS{});
    lines.init(IDX_VERTEX);
    lines.update();

    Input input;
    input.setup(window);

    EditorState state;
    state.selectedRef = 0;

    static UndoStack undoStack;

    std::filesystem::path modelsDir = exePath.parent_path() / "models";
    if (!scene::load_models(triangles, state, modelsDir))
    {
        printf("Failed to load scene\n");
        return 1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        editor::process_input(input, state, triangles, undoStack);
        if (state.shouldQuit)
        {
            break;
        }
        input.reset();

        bool hasObj = triangles.usedSlots[state.selectedRef];
        Mat4 objMat;

        if (hasObj)
        {
            objMat = trs::compose(triangles.transforms[state.selectedRef]);
            TRS axisTRS = triangles.transforms[state.selectedRef];
            axisTRS.sx = axisTRS.sy = axisTRS.sz = 0.8f;
            axisMat = trs::compose(axisTRS);
        }
        else
        {
            axisMat = mat4::IDENTITY;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, state.wireframe ? GL_LINE : GL_FILL);

        if (hasObj)
        {
            triangles.models[state.selectedRef] = objMat;
            triangles.update_models();

            highlights.reset();
            editor::build_highlights(state, triangles, state.selectedRef, highlights, objMat);
            highlights.update();
        }

        triangles.draw();

        // change depth func to render highlights on top
        glLineWidth(3.0f);
        glDepthFunc(GL_LEQUAL);
        highlights.draw();
        glDepthFunc(GL_LESS);
        glLineWidth(1.0f);

        lines.models[axRef] = axisMat;
        lines.models[ayRef] = axisMat;
        lines.models[azRef] = axisMat;
        lines.update_models();
        lines.draw();

        glfwSwapBuffers(window);
    }

    triangles.free();
    highlights.free();
    lines.free();
    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
