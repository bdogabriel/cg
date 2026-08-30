#include "buffer.h" // has to be included first

#include "binding.h"
#include "renderer.h"
#include "scene.h"
#include "ui.h"

constexpr GLuint IDX_VERTEX = 0;

void load_shader(GLuint program, const char *code, GLenum type)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        printf("E: shader compilation failed: %s\n", infoLog);
        exit(1);
    }

    glAttachShader(program, shader);
}

GLFWwindow *setup_window()
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Handmade", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glewInit();

    glfwShowWindow(window);

    return window;
}

static bool read_file(const char *path, char *out, size_t outSize)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        printf("E: cannot open %s\n", path);
        return false;
    }
    size_t n = fread(out, 1, outSize - 1, f);
    out[n] = '\0';
    fclose(f);
    return true;
}

GLuint setup_program()
{
    GLuint program = glCreateProgram();

    char vSrc[4096];
    char fSrc[4096];
    if (!read_file("shader/vertex.glsl", vSrc, sizeof(vSrc)) || !read_file("shader/fragment.glsl", fSrc, sizeof(fSrc)))
    {
        exit(1);
    }

    load_shader(program, vSrc, GL_VERTEX_SHADER);
    load_shader(program, fSrc, GL_FRAGMENT_SHADER);

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

void setup_bindings()
{
    binding::bind(Key::ESC, mods::NONE, true, "set_cmd", "op=none");
    binding::bind(Key::T, mods::NONE, true, "set_cmd", "op=translate");
    binding::bind(Key::R, mods::NONE, true, "set_cmd", "op=rotate");
    binding::bind(Key::S, mods::NONE, true, "set_cmd", "op=scale");
    binding::bind(Key::A, mods::NONE, true, "set_cmd", "op=shear");
    binding::bind(Key::E, mods::NONE, true, "set_cmd", "op=extrude");
    binding::bind(Key::D, mods::NONE, true, "set_cmd", "op=delete");
    binding::bind(Key::M, mods::NONE, true, "set_cmd", "op=merge");
    binding::bind(Key::F, mods::NONE, true, "set_target", "t=face");
    binding::bind(Key::O, mods::NONE, true, "set_target", "t=object");
    binding::bind(Key::X, mods::NONE, true, "set_lock", "axis=x");
    binding::bind(Key::Y, mods::NONE, true, "set_lock", "axis=y");
    binding::bind(Key::Z, mods::NONE, true, "set_lock", "axis=z");
    binding::bind(Key::H, mods::NONE, false, "set_args", "axis=x step=-1");
    binding::bind(Key::L, mods::NONE, false, "set_args", "axis=x step=1");
    binding::bind(Key::J, mods::NONE, false, "set_args", "axis=y step=-1");
    binding::bind(Key::K, mods::NONE, false, "set_args", "axis=y step=1");
    binding::bind(Key::U, mods::CTRL, false, "set_args", "axis=z step=1");
    binding::bind(Key::D, mods::CTRL, false, "set_args", "axis=z step=-1");
    binding::bind(Key::SPACE, mods::NONE, true, "toggle_selection", "");
    binding::bind(Key::C, mods::NONE, true, "clear_selection", "");
    binding::bind(Key::N, mods::NONE, true, "cycle", "step=1");
    binding::bind(Key::P, mods::NONE, true, "cycle", "step=-1");
    binding::bind(Key::W, mods::NONE, true, "wireframe_toggle", "");
    binding::bind(Key::U, mods::NONE, true, "undo", "");
    binding::bind(Key::R, mods::SHIFT, true, "reset", "property=rotation");
    binding::bind(Key::T, mods::SHIFT, true, "reset", "property=translation");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    GLFWwindow *window = setup_window();
    GLuint program = setup_program();
    setup_graphics();

    // holds the undo history (~25 MB); static so it lives in BSS, not the stack
    static Editor editor;

    Buffer meshes;
    Buffer highlights;
    Buffer axes;

    Renderer meshRenderer;
    Renderer highlightRenderer;
    highlightRenderer.primitive = GL_LINES;
    Renderer axesRenderer;
    axesRenderer.primitive = GL_LINES;

    render::init(meshRenderer, IDX_VERTEX);
    render::init(highlightRenderer, IDX_VERTEX);
    render::init(axesRenderer, IDX_VERTEX);

    Input input;
    input::setup(input, window);
    editor::setup(editor, input);

    setup_bindings();

    if (!scene::load_models(editor.buffer, editor))
    {
        printf("Failed to load scene\n");
        return 1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        editor::process_input(editor);
        if (editor.shouldQuit)
        {
            break;
        }
        input::reset(input);

        if (editor.buffer.meshDirty)
        {
            render::upload_mesh(meshRenderer, editor.buffer);
            render::upload_commands(meshRenderer, editor.buffer);
            render::upload_face_colors(meshRenderer, editor.buffer, 3);
            editor.buffer.meshDirty = false;
        }
        if (editor.buffer.modelsDirty)
        {
            render::upload_models(meshRenderer, editor.buffer);
            editor.buffer.modelsDirty = false;
        }

        ui::build_overlays(editor, highlights, axes);

        render::upload_mesh(highlightRenderer, highlights);
        render::upload_commands(highlightRenderer, highlights);
        render::upload_models(highlightRenderer, highlights);
        render::upload_face_colors(highlightRenderer, highlights, 2);

        render::upload_mesh(axesRenderer, axes);
        render::upload_commands(axesRenderer, axes);
        render::upload_models(axesRenderer, axes);
        render::upload_face_colors(axesRenderer, axes, 2);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, editor.wireframe ? GL_LINE : GL_FILL);

        render::draw(meshRenderer);

        glLineWidth(3.0f);
        glDepthFunc(GL_LEQUAL);
        render::draw(highlightRenderer);
        glDepthFunc(GL_LESS);
        glLineWidth(1.0f);

        render::draw(axesRenderer);

        glfwSwapBuffers(window);
    }

    render::free(meshRenderer);
    render::free(highlightRenderer);
    render::free(axesRenderer);
    glDeleteProgram(program);
    glfwTerminate();

    return 0;
}
