#include "renderer.h"

#include "camera.h"
#include "gl_skybox.h"
#include "gl_util.h"
#include "model_texture_cache.h"
#include "render_1d.h"
#include "render_2d.h"
#include "render_3d.h"
#include "window.h"

#include <GL/glew.h>

#include <stdio.h>

namespace render
{

namespace
{

struct Context
{
    Window *window = nullptr;

    Render3D meshRenderer;
    Render2D hudRenderer;
    Render1D lineRenderer;
    glskybox::Skybox skybox;

    GLuint meshProgram = 0;
    GLuint meshTexturedProgram = 0;
    GLuint quadTexturedR8Program = 0;
    GLuint quadProgram = 0;
    GLuint lineProgram = 0;
    GLuint skyboxProgram = 0;

    GLint u_meshViewProj = -1;
    GLint u_meshTexturedViewProj = -1;
    GLint u_lineViewProj = -1;
    GLint u_skyboxViewProj = -1;
    GLint u_quadTexturedR8FbSize = -1;
    GLint u_quadFbSize = -1;

    Mat4 viewProj;
    Mat4 skyViewProj;

    int fbWidth = 0;
    int fbHeight = 0;
};

Context g_context;
bool g_initialized = false;

void setup_graphics()
{
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void reset_context()
{
    g_context = Context{};
}

} // namespace

bool init(Window &window)
{
    if (g_initialized)
    {
        return false;
    }

    reset_context();
    g_context.window = &window;

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        // on Wayland/EGL GLEW's GLX layer can report NO_GLX_DISPLAY;
        // even though the GL context and core entry points are valid
        if (glewErr == GLEW_ERROR_NO_GLX_DISPLAY)
        {
            fprintf(stderr, "W: GLEW has no GLX display (Wayland/EGL); continuing with loaded GL entry points\n");
        }
        else
        {
            fprintf(stderr, "W: glewInit failed (code %u: %s); continuing\n", (unsigned)glewErr,
                    glewGetErrorString(glewErr));
        }
    }

    if (glGetString(GL_VERSION) == nullptr)
    {
        fprintf(stderr, "E: no current OpenGL context\n");
        reset_context();
        return false;
    }

    if (!glCreateShader || !glCreateBuffers || !glCreateVertexArrays || !glMultiDrawElementsIndirect ||
        !glCreateSamplers || !glBindSampler)
    {
        fprintf(stderr, "E: required OpenGL entry points are missing\n");
        reset_context();
        return false;
    }

    setup_graphics();

    g_context.meshProgram = glutil::make_program("shader/gl/mesh_vertex.glsl", "shader/gl/mesh_fragment.glsl");
    g_context.meshTexturedProgram =
        glutil::make_program("shader/gl/mesh_textured_vertex.glsl", "shader/gl/mesh_textured_fragment.glsl");
    g_context.quadTexturedR8Program =
        glutil::make_program("shader/gl/quad_textured_r8_vertex.glsl", "shader/gl/quad_textured_r8_fragment.glsl");
    g_context.quadProgram = glutil::make_program("shader/gl/quad_vertex.glsl", "shader/gl/quad_fragment.glsl");
    g_context.lineProgram = glutil::make_program("shader/gl/line_vertex.glsl", "shader/gl/line_fragment.glsl");
    g_context.skyboxProgram = glutil::make_program("shader/gl/skybox_vertex.glsl", "shader/gl/skybox_fragment.glsl");

    g_context.u_meshViewProj = glGetUniformLocation(g_context.meshProgram, "u_viewProj");
    g_context.u_meshTexturedViewProj = glGetUniformLocation(g_context.meshTexturedProgram, "u_viewProj");
    g_context.u_lineViewProj = glGetUniformLocation(g_context.lineProgram, "u_viewProj");
    g_context.u_skyboxViewProj = glGetUniformLocation(g_context.skyboxProgram, "u_viewProj");
    g_context.u_quadTexturedR8FbSize = glGetUniformLocation(g_context.quadTexturedR8Program, "fbSize");
    g_context.u_quadFbSize = glGetUniformLocation(g_context.quadProgram, "fbSize");

    GLint meshTexturedSampler = glGetUniformLocation(g_context.meshTexturedProgram, "u_tex");
    glProgramUniform1i(g_context.meshTexturedProgram, meshTexturedSampler, 0);
    GLint quadTexturedR8Sampler = glGetUniformLocation(g_context.quadTexturedR8Program, "tex");
    glProgramUniform1i(g_context.quadTexturedR8Program, quadTexturedR8Sampler, 0);
    GLint skyboxSampler = glGetUniformLocation(g_context.skyboxProgram, "u_skybox");
    glProgramUniform1i(g_context.skyboxProgram, skyboxSampler, 0);

    render3d::init(g_context.meshRenderer, 0);
    render2d::init(g_context.hudRenderer);
    render1d::init(g_context.lineRenderer);
    glskybox::init(g_context.skybox);

    g_initialized = true;
    return true;
}

void shutdown()
{
    if (!g_initialized)
    {
        return;
    }

    render3d::free(g_context.meshRenderer);
    render1d::free(g_context.lineRenderer);
    render2d::free(g_context.hudRenderer);
    glskybox::free(g_context.skybox);

    glDeleteProgram(g_context.meshProgram);
    glDeleteProgram(g_context.meshTexturedProgram);
    glDeleteProgram(g_context.quadTexturedR8Program);
    glDeleteProgram(g_context.quadProgram);
    glDeleteProgram(g_context.lineProgram);
    glDeleteProgram(g_context.skyboxProgram);

    reset_context();
    g_initialized = false;
}

bool needs_graphics_context()
{
    return true;
}

void begin_frame(int fbWidth, int fbHeight, Color clear)
{
    g_context.fbWidth = fbWidth;
    g_context.fbHeight = fbHeight;

    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glClearColor(clear.r / 255.0f, clear.g / 255.0f, clear.b / 255.0f, clear.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void set_camera(const Camera &camera, float aspect)
{
    Mat4 projection = camera::projection(camera, aspect);
    g_context.viewProj = projection * camera::view(camera);
    g_context.skyViewProj = projection * camera::view_rotation(camera);
}

void sync_mesh(MeshBatch &batch)
{
    if (batch.meshDirty)
    {
        render3d::upload_mesh(g_context.meshRenderer, batch);
        render3d::upload_commands(g_context.meshRenderer, batch);
        render3d::upload_face_colors(g_context.meshRenderer, batch);
        batch.meshDirty = false;
    }
    if (batch.modelsDirty)
    {
        render3d::upload_models(g_context.meshRenderer, batch);
        batch.modelsDirty = false;
    }
}

void draw_meshes(const MeshBatch &batch, const ModelTextureCache &textures, DrawMode mode)
{
    glPolygonMode(GL_FRONT_AND_BACK, mode == DrawMode::WIREFRAME ? GL_LINE : GL_FILL);

    glUseProgram(g_context.meshProgram);
    glUniformMatrix4fv(g_context.u_meshViewProj, 1, GL_FALSE, g_context.viewProj.data());
    render3d::draw_flat(g_context.meshRenderer, batch);

    glUseProgram(g_context.meshTexturedProgram);
    glUniformMatrix4fv(g_context.u_meshTexturedViewProj, 1, GL_FALSE, g_context.viewProj.data());
    render3d::draw_textured(g_context.meshRenderer, batch, textures);
}

void draw_lines(const LineBatch &batch, LineStyle style)
{
    if (batch.vtxCount == 0)
    {
        return;
    }

    glUseProgram(g_context.lineProgram);
    glUniformMatrix4fv(g_context.u_lineViewProj, 1, GL_FALSE, g_context.viewProj.data());
    glLineWidth(style.width);
    glDepthFunc(style.depthEqual ? GL_LEQUAL : GL_LESS);
    render1d::draw(g_context.lineRenderer, batch);
    glDepthFunc(GL_LESS);
    glLineWidth(1.0f);
}

void draw_quads(const QuadBatch &batch, QuadPipeline pipeline, TextureHandle texture)
{
    if (batch.indexCount == 0)
    {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (pipeline == QuadPipeline::TEXT)
    {
        glUseProgram(g_context.quadTexturedR8Program);
        glUniform2f(g_context.u_quadTexturedR8FbSize, (float)g_context.fbWidth, (float)g_context.fbHeight);
        glBindTextureUnit(0, texture);
    }
    else
    {
        glUseProgram(g_context.quadProgram);
        glUniform2f(g_context.u_quadFbSize, (float)g_context.fbWidth, (float)g_context.fbHeight);
    }

    render2d::draw(g_context.hudRenderer, batch);
}

void draw_skybox(TextureHandle texture)
{
    if (texture == 0)
    {
        return;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(g_context.skyboxProgram);
    glUniformMatrix4fv(g_context.u_skyboxViewProj, 1, GL_FALSE, g_context.skyViewProj.data());
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glskybox::draw(g_context.skybox, texture);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void end_frame()
{
    window::swap_buffers(*g_context.window);
}

TextureHandle create_texture(const TextureDesc &desc)
{
    if (desc.width <= 0 || desc.height <= 0 || desc.pixels == nullptr)
    {
        return 0;
    }

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (desc.width > maxSize || desc.height > maxSize)
    {
        fprintf(stderr, "E: texture %dx%d exceeds GL_MAX_TEXTURE_SIZE (%d)\n", desc.width, desc.height, (int)maxSize);
        return 0;
    }

    GLenum internalFormat = (desc.format == TextureFormat::R8) ? GL_R8 : GL_RGBA8;
    GLenum format = (desc.format == TextureFormat::R8) ? GL_RED : GL_RGBA;
    return (TextureHandle)glutil::make_texture(desc.width, desc.height, (const uint8_t *)desc.pixels, internalFormat,
                                               format);
}

void destroy_texture(TextureHandle handle)
{
    if (handle == 0)
    {
        return;
    }
    GLuint texture = (GLuint)handle;
    glDeleteTextures(1, &texture);
}

} // namespace render
