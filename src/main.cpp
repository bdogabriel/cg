#include "renderer.h"

#include "arena.h"
#include "binding.h"
#include "command.h"
#include "dispatch.h"
#include "editor.h"
#include "font.h"
#include "model_texture_cache.h"
#include "ui_2d.h"
#include "ui_3d.h"
#include "window.h"

namespace
{

bool setup_window(Window &w)
{
    WindowConfig config;
    config.width = 1000;
    config.height = 1000;
    config.title = "Editor";
    config.createGraphicsContext = render::needs_graphics_context();
    return window::create(w, config);
}

} // namespace

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
    binding::bind(Key::F, mods::NONE, true, "set_target", "target=face");
    binding::bind(Key::O, mods::NONE, true, "set_target", "target=object");
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
    binding::bind(Key::S, mods::SHIFT, true, "reset", "property=scale");
    binding::bind(Key::S, mods::ALT, true, "set_cmd", "op=uniform_scale");
    binding::bind(Key::SEMICOLON, mods::NONE, true, "toggle_lock", "");
    binding::bind(Key::SEMICOLON, mods::SHIFT, true, "prompt", "");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    static Editor editor;
    if (!setup_window(editor.window))
    {
        return 1;
    }

    if (!render::init(editor.window))
    {
        window::destroy(editor.window);
        return 1;
    }
    window::show(editor.window);

    mesh::reset(editor.meshBatch);

    QuadBatch textBatch;
    QuadBatch solidBatch;

    LineBatch highlightLines;
    LineBatch axesLines;

    const Font &font = baked_font();
    render::TextureHandle atlasTexture =
        render::create_texture({font.atlasWidth, font.atlasHeight, render::TextureFormat::R8, baked_atlas()});

    keyboard::setup(editor.keyboard, editor.window);

    setup_bindings();
    editor.cmd = cmd::find("camera");

    while (!window::should_close(editor.window))
    {
        arena::scratch_reset();

        window::poll_events();
        input::dispatch(editor);
        if (window::should_close(editor.window))
        {
            break;
        }
        keyboard::reset(editor.keyboard);

        int fbW = editor.window.fbWidth;
        int fbH = editor.window.fbHeight;

        if (editor.meshBatch.meshDirty)
        {
            mesh::triangulate(editor.meshBatch);
            mesh::build_commands(editor.meshBatch, modeltexture::count(editor.modelTextureCache));
        }
        render::sync_mesh(editor.meshBatch);

        line::reset(axesLines);
        line::reset(highlightLines);

        ui3d::build_axes(editor, axesLines);
        ui3d::build_face_highlights(editor, highlightLines);

        quad::reset(textBatch);
        quad::reset(solidBatch);
        ui2d::build_statusline(editor, fbW, fbH, textBatch, solidBatch);
        if (cmd::is_exec(editor))
        {
            ui2d::build_prompt(editor, fbW, fbH, textBatch, solidBatch);
        }

        float aspect = (fbH > 0) ? (float)fbW / (float)fbH : 1.0f;

        render::begin_frame(fbW, fbH, editor.bgColor);
        render::set_camera(editor.camera, aspect);
        render::draw_meshes(editor.meshBatch, editor.modelTextureCache,
                            editor.wireframe ? render::DrawMode::WIREFRAME : render::DrawMode::SHADED);
        render::draw_lines(highlightLines, {3.0f, true});
        render::draw_lines(axesLines, {1.0f, false});
        render::draw_skybox(editor.skyboxTexture);
        render::draw_quads(solidBatch, render::QuadPipeline::SOLID, 0);
        render::draw_quads(textBatch, render::QuadPipeline::TEXT, atlasTexture);
        render::end_frame();
    }

    render::destroy_texture(atlasTexture);
    modeltexture::reset(editor.modelTextureCache);
    render::destroy_texture(editor.skyboxTexture);
    editor.skyboxTexture = 0;
    render::shutdown();
    window::destroy(editor.window);

    return 0;
}
