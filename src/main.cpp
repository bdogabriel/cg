#include "gl_util.h"
#include "mesh_batch.h"
#include "render_1d.h"

#include "binding.h"
#include "camera.h"
#include "font.h"
#include "render_2d.h"
#include "render_3d.h"
#include "session.h"
#include "ui_2d.h"
#include "ui_3d.h"

constexpr GLuint IDX_VERTEX = 0;

namespace
{

int g_fbW = 1000;
int g_fbH = 1000;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    g_fbW = width;
    g_fbH = height;
    glViewport(0, 0, width, height);
}

} // namespace

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
    binding::bind(Key::ESC, mods::NONE, true, "set_cmd", "op=camera");
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
    GLuint meshProgram = glutil::make_program("shader/mesh_vertex.glsl", "shader/mesh_fragment.glsl");
    GLuint meshTexturedProgram =
        glutil::make_program("shader/mesh_textured_vertex.glsl", "shader/mesh_textured_fragment.glsl");
    glUseProgram(meshProgram);
    GLuint quadTexturedR8Program =
        glutil::make_program("shader/quad_textured_r8_vertex.glsl", "shader/quad_textured_r8_fragment.glsl");
    GLint u_quadTexturedR8FbSize = glGetUniformLocation(quadTexturedR8Program, "fbSize");
    GLint u_quadTexturedR8Sampler = glGetUniformLocation(quadTexturedR8Program, "tex");
    glProgramUniform1i(quadTexturedR8Program, u_quadTexturedR8Sampler, 0);
    GLuint quadProgram = glutil::make_program("shader/quad_vertex.glsl", "shader/quad_fragment.glsl");
    GLint u_quadFbSize = glGetUniformLocation(quadProgram, "fbSize");
    GLuint lineProgram = glutil::make_program("shader/line_vertex.glsl", "shader/line_fragment.glsl");
    GLint u_meshViewProj = glGetUniformLocation(meshProgram, "u_viewProj");
    GLint u_meshTexturedViewProj = glGetUniformLocation(meshTexturedProgram, "u_viewProj");
    GLint u_meshTexturedSampler = glGetUniformLocation(meshTexturedProgram, "u_tex");
    glProgramUniform1i(meshTexturedProgram, u_meshTexturedSampler, 0);
    GLint u_lineViewProj = glGetUniformLocation(lineProgram, "u_viewProj");
    setup_graphics();

    static Editor editor;
    static Session session;

    Render3D meshRenderer;
    Render2D hudRenderer;
    QuadBatch textBatch;
    QuadBatch solidBatch;

    LineBatch highlightLines;
    LineBatch axesLines;
    Render1D lineRenderer;

    const Font &font = baked_font();
    GLuint atlasTexture = glutil::make_texture(font.atlasWidth, font.atlasHeight, baked_atlas(), GL_R8, GL_RED);

    render3d::init(meshRenderer, IDX_VERTEX);
    render2d::init(hudRenderer);
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
            mesh::build_commands(editor.meshBatch, modeltexture::count(editor.modelTextureCache));
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

        glClearColor(editor.bgColor.r / 255.0f, editor.bgColor.g / 255.0f, editor.bgColor.b / 255.0f,
                     editor.bgColor.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, editor.wireframe ? GL_LINE : GL_FILL);

        float aspect = (g_fbH > 0) ? (float)g_fbW / (float)g_fbH : 1.0f;
        Mat4 viewProj = camera::projection(editor.camera, aspect) * camera::view(editor.camera);

        glUseProgram(meshProgram);
        glUniformMatrix4fv(u_meshViewProj, 1, GL_FALSE, viewProj.data());
        render3d::draw_flat(meshRenderer, editor.meshBatch);

        glUseProgram(meshTexturedProgram);
        glUniformMatrix4fv(u_meshTexturedViewProj, 1, GL_FALSE, viewProj.data());
        render3d::draw_textured(meshRenderer, editor.meshBatch, editor.modelTextureCache);

        glUseProgram(lineProgram);
        glUniformMatrix4fv(u_lineViewProj, 1, GL_FALSE, viewProj.data());
        glLineWidth(3.0f);
        glDepthFunc(GL_LEQUAL);
        render1d::draw(lineRenderer, highlightLines);
        glDepthFunc(GL_LESS);
        glLineWidth(1.0f);
        render1d::draw(lineRenderer, axesLines);

        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        quad::reset(textBatch);
        quad::reset(solidBatch);
        ui2d::build_statusline(editor, session, g_fbW, g_fbH, textBatch, solidBatch);
        if (session::is_cmd_exec(session))
        {
            ui2d::build_prompt(session, g_fbW, g_fbH, textBatch, solidBatch);
        }
        glUseProgram(quadProgram);
        glUniform2f(u_quadFbSize, (float)g_fbW, (float)g_fbH);
        render2d::draw(hudRenderer, solidBatch);
        glUseProgram(quadTexturedR8Program);
        glUniform2f(u_quadTexturedR8FbSize, (float)g_fbW, (float)g_fbH);
        glBindTextureUnit(0, atlasTexture);
        render2d::draw(hudRenderer, textBatch);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
    }

    render3d::free(meshRenderer);
    render1d::free(lineRenderer);
    render2d::free(hudRenderer);
    glDeleteProgram(meshProgram);
    glDeleteProgram(meshTexturedProgram);
    glDeleteProgram(quadTexturedR8Program);
    glDeleteProgram(quadProgram);
    glDeleteProgram(lineProgram);
    glDeleteTextures(1, &atlasTexture);
    glfwTerminate();

    return 0;
}
