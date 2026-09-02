#include "gl_util.h"
#include "meshbatch.h"
#include "render1d.h"

#include "binding.h"
#include "font.h"
#include "render2d.h"
#include "render3d.h"
#include "session.h"
#include "ui2d.h"
#include "ui3d.h"
#include <cstdio>

constexpr GLuint IDX_VERTEX = 0;
static int g_fbW = 1000;
static int g_fbH = 1000;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    g_fbW = width;
    g_fbH = height;
    glViewport(0, 0, width, height);
}

GLFWwindow *setup_window()
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Handmade", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glViewport(0, 0, g_fbW, g_fbH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glewInit();

    glfwShowWindow(window);

    return window;
}

void setup_graphics()
{
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void setup_bindings()
{
    binding::bind(Key::ESC, mods::NONE, true, "set_cmd", "op=none");
    binding::bind(Key::T, mods::NONE, true, "set_cmd", "op=translate");
    binding::bind(Key::R, mods::NONE, true, "set_cmd", "op=rotate");
    binding::bind(Key::S, mods::NONE, true, "set_cmd", "op=scale");
    binding::bind(Key::A, mods::NONE, true, "set_cmd", "op=shear");
    binding::bind(Key::E, mods::NONE, true, "set_cmd", "op=extrude");
    binding::bind(Key::D, mods::CTRL, true, "set_cmd", "op=delete");
    binding::bind(Key::M, mods::NONE, true, "set_cmd", "op=merge");
    binding::bind(Key::F, mods::NONE, true, "set_target", "t=face");
    binding::bind(Key::O, mods::NONE, true, "set_target", "t=object");
    binding::bind(Key::H, mods::NONE, false, "set_args", "axis=x step=-1");
    binding::bind(Key::L, mods::NONE, false, "set_args", "axis=x step=1");
    binding::bind(Key::J, mods::NONE, false, "set_args", "axis=y step=-1");
    binding::bind(Key::K, mods::NONE, false, "set_args", "axis=y step=1");
    binding::bind(Key::U, mods::NONE, false, "set_args", "axis=z step=1");
    binding::bind(Key::D, mods::NONE, false, "set_args", "axis=z step=-1");
    binding::bind(Key::SPACE, mods::NONE, true, "toggle_selection", "");
    binding::bind(Key::C, mods::NONE, true, "apply_color", "");
    binding::bind(Key::N, mods::NONE, true, "cycle", "step=1");
    binding::bind(Key::P, mods::NONE, true, "cycle", "step=-1");
    binding::bind(Key::W, mods::NONE, true, "wireframe_toggle", "");
    binding::bind(Key::U, mods::CTRL, true, "undo", "");
    binding::bind(Key::R, mods::SHIFT, true, "reset", "property=rotation");
    binding::bind(Key::T, mods::SHIFT, true, "reset", "property=translation");
    binding::bind(Key::SEMICOLON, mods::NONE, true, "toggle_lock", "");
    binding::bind(Key::SEMICOLON, mods::SHIFT, true, "prompt", "");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    GLFWwindow *window = setup_window();
    GLuint program = glutil::make_program("shader/mesh_vertex.glsl", "shader/mesh_fragment.glsl");
    glUseProgram(program);
    GLuint textProgram = glutil::make_program("shader/text_vertex.glsl", "shader/text_fragment.glsl");
    GLint u_fbSize = glGetUniformLocation(textProgram, "fbSize");
    GLint sampler = glGetUniformLocation(textProgram, "atlas");
    glProgramUniform1i(textProgram, sampler, 0);
    GLuint lineProgram = glutil::make_program("shader/line_vertex.glsl", "shader/line_fragment.glsl");
    setup_graphics();

    static Editor editor;
    static Session session;

    Render3d meshRenderer;
    Render2d hudRenderer;
    QuadBatch hudBatch;

    LineBatch highlightLines;
    LineBatch axesLines;
    Render1d lineRenderer;

    render3d::init(meshRenderer, IDX_VERTEX);
    render2d::init(hudRenderer, baked_font(), baked_atlas());
    render1d::init(lineRenderer);

    Keyboard keyboard;
    keyboard::setup(keyboard, window);
    session::setup(session, keyboard);

    setup_bindings();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        session::process_input(editor, session);
        if (session.shouldQuit)
        {
            break;
        }
        keyboard::reset(keyboard);

        if (editor.meshBatch.meshDirty)
        {
            mesh::triangulate(editor.meshBatch);
            render3d::upload_mesh(meshRenderer, editor.meshBatch);
            render3d::upload_commands(meshRenderer, editor.meshBatch);
            render3d::upload_face_colors(meshRenderer, editor.meshBatch);
            editor.meshBatch.meshDirty = false;
        }
        if (editor.meshBatch.modelsDirty)
        {
            render3d::upload_models(meshRenderer, editor.meshBatch);
            editor.meshBatch.modelsDirty = false;
        }

        line::reset(axesLines);
        line::reset(highlightLines);

        ui3d::build_axes(editor, axesLines);
        ui3d::build_face_highlights(editor, highlightLines);

        glClearColor(editor.bgColor.r / 255.0f, editor.bgColor.g / 255.0f,
                     editor.bgColor.b / 255.0f, editor.bgColor.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, editor.wireframe ? GL_LINE : GL_FILL);

        glUseProgram(program);
        render3d::draw(meshRenderer);

        glUseProgram(lineProgram);
        glLineWidth(3.0f);
        glDepthFunc(GL_LEQUAL);
        render1d::draw(lineRenderer, highlightLines);
        glDepthFunc(GL_LESS);
        glLineWidth(1.0f);
        render1d::draw(lineRenderer, axesLines);

        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        quad::reset(hudBatch);
        ui2d::build_statusline(editor, session, g_fbW, g_fbH, hudBatch);
        if (session::is_cmd_exec(session))
        {
            ui2d::build_prompt(session, g_fbW, g_fbH, hudBatch);
        }
        glUseProgram(textProgram);
        glUniform2f(u_fbSize, (float)g_fbW, (float)g_fbH);
        render2d::draw(hudRenderer, hudBatch);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
    }

    render3d::free(meshRenderer);
    render1d::free(lineRenderer);
    render2d::free(hudRenderer);
    glDeleteProgram(program);
    glDeleteProgram(textProgram);
    glDeleteProgram(lineProgram);
    glfwTerminate();

    return 0;
}
