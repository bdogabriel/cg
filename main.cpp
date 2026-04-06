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
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Editor", NULL, NULL);
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
    TRS cubeTRS;
    cubeTRS.sx = cubeTRS.sy = cubeTRS.sz = 0.3f;
    Ref cubeRef = triangles.add(geo::cube, cubeTRS);
    triangles.init(IDX_VERTEX);
    triangles.update();

    DrawBuffer highlight;
    highlight.primitive = GL_LINES;
    highlight.init(IDX_VERTEX);

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
    state.selectedRef = cubeRef;

    static UndoStack undoStack;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        editor::process_input(input, state, triangles, undoStack);
        input.reset();

        Mat4 cubeMat = trs::compose(triangles.transforms[cubeRef]);
        TRS axisTRS = triangles.transforms[cubeRef];
        axisTRS.sx = axisTRS.sy = axisTRS.sz = 0.8f;
        axisMat = trs::compose(axisTRS);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, state.wireframe ? GL_LINE : GL_FILL);

        triangles.models[cubeRef] = cubeMat;
        triangles.update_models();

        highlight.vtxCount = 0;
        highlight.idxCount = 0;
        highlight.objCount = 1;

        if (state.mode >= Mode::TRANSLATE_FACE)
        {
            DrawCommand &cmd = triangles.commands[cubeRef];
            auto addFaceEdges = [&](int face, Color color) {
                int base = cmd.indexOffset + face * 3;
                Vec4 v[3] = {
                    triangles.vertices[cmd.vertexOffset + triangles.indices[base + 0]],
                    triangles.vertices[cmd.vertexOffset + triangles.indices[base + 1]],
                    triangles.vertices[cmd.vertexOffset + triangles.indices[base + 2]],
                };
                unsigned int idx[6] = {0, 1, 1, 2, 2, 0};
                Color c[3] = {color, color, color};
                Ref r = highlight.add({v, 3, idx, 6, c, 3}, TRS{});
                highlight.models[r] = cubeMat;
            };

            addFaceEdges(state.faceCursor, {1.0f, 0.8f, 0.0f, 1});
            for (int i = 0; i < state.selectedFaceCount; i++)
            {
                addFaceEdges(state.selectedFaces[i], {1.0f, 0.4f, 0.0f, 1});
            }
        }
        highlight.update();

        triangles.draw();

        glLineWidth(3.0f);
        glDepthFunc(GL_LEQUAL);
        highlight.draw();
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
    highlight.free();
    lines.free();
    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
