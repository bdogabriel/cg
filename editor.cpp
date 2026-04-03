#include "editor.h"
#include "trs.h"
#include <GLFW/glfw3.h>

static const Binding bindings[] = {
    {Mode::ANY, GLFW_MOUSE_BUTTON_RIGHT, 0,
     [](EditorState &s, const Input &in) {
         trs::translate(*s.selected, (float)in.mouseDeltaX * s.cfg.mouseSensitivity,
                        (float)in.mouseDeltaY * -s.cfg.mouseSensitivity, 0);
     }},
    {Mode::ANY, KEY_SCROLL_UP, 0,
     [](EditorState &s, const Input &in) {
         float f = 1.0f + (float)in.scrollDeltaY * s.cfg.scrollSensitivity;
         trs::scale(*s.selected, f, f, f);
     }},
    {Mode::ANY, KEY_SCROLL_DOWN, 0,
     [](EditorState &s, const Input &in) {
         float f = 1.0f + (float)in.scrollDeltaY * s.cfg.scrollSensitivity;
         trs::scale(*s.selected, f, f, f);
     }},

    {Mode::ANY, GLFW_KEY_T, 0, [](EditorState &s, const Input &) { s.mode = Mode::TRANSLATE_LOCAL; }},
    {Mode::ANY, GLFW_KEY_T, GLFW_MOD_SHIFT, [](EditorState &s, const Input &) { s.mode = Mode::TRANSLATE_WORLD; }},
    {Mode::ANY, GLFW_KEY_R, 0, [](EditorState &s, const Input &) { s.mode = Mode::ROTATE_LOCAL; }},
    {Mode::ANY, GLFW_KEY_R, GLFW_MOD_SHIFT, [](EditorState &s, const Input &) { s.mode = Mode::ROTATE_WORLD; }},
    {Mode::ANY, GLFW_KEY_S, 0, [](EditorState &s, const Input &) { s.mode = Mode::SCALE; }},

    {Mode::TRANSLATE_LOCAL, GLFW_KEY_H, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_LOCAL, GLFW_KEY_L, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_LOCAL, GLFW_KEY_K, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, 0, s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_LOCAL, GLFW_KEY_J, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_LOCAL, GLFW_KEY_N, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, 0, 0, s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_LOCAL, GLFW_KEY_P, 0,
     [](EditorState &s, const Input &) { trs::translate_local(*s.selected, 0, 0, -s.cfg.keySensitivity); }},

    {Mode::TRANSLATE_WORLD, GLFW_KEY_H, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_WORLD, GLFW_KEY_L, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_WORLD, GLFW_KEY_K, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, 0, s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_WORLD, GLFW_KEY_J, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_WORLD, GLFW_KEY_N, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, 0, 0, s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_WORLD, GLFW_KEY_P, 0,
     [](EditorState &s, const Input &) { trs::translate(*s.selected, 0, 0, -s.cfg.keySensitivity); }},

    {Mode::ROTATE_LOCAL, GLFW_KEY_H, 0,
     [](EditorState &s, const Input &) { trs::rotate_y_local(*s.selected, -s.cfg.keySensitivity); }},
    {Mode::ROTATE_LOCAL, GLFW_KEY_L, 0,
     [](EditorState &s, const Input &) { trs::rotate_y_local(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_LOCAL, GLFW_KEY_K, 0,
     [](EditorState &s, const Input &) { trs::rotate_x_local(*s.selected, -s.cfg.keySensitivity); }},
    {Mode::ROTATE_LOCAL, GLFW_KEY_J, 0,
     [](EditorState &s, const Input &) { trs::rotate_x_local(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_LOCAL, GLFW_KEY_N, 0,
     [](EditorState &s, const Input &) { trs::rotate_z_local(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_LOCAL, GLFW_KEY_P, 0,
     [](EditorState &s, const Input &) { trs::rotate_z_local(*s.selected, -s.cfg.keySensitivity); }},

    {Mode::ROTATE_WORLD, GLFW_KEY_H, 0,
     [](EditorState &s, const Input &) { trs::rotate_y(*s.selected, -s.cfg.keySensitivity); }},
    {Mode::ROTATE_WORLD, GLFW_KEY_L, 0,
     [](EditorState &s, const Input &) { trs::rotate_y(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_WORLD, GLFW_KEY_K, 0,
     [](EditorState &s, const Input &) { trs::rotate_x(*s.selected, -s.cfg.keySensitivity); }},
    {Mode::ROTATE_WORLD, GLFW_KEY_J, 0,
     [](EditorState &s, const Input &) { trs::rotate_x(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_WORLD, GLFW_KEY_N, 0,
     [](EditorState &s, const Input &) { trs::rotate_z(*s.selected, s.cfg.keySensitivity); }},
    {Mode::ROTATE_WORLD, GLFW_KEY_P, 0,
     [](EditorState &s, const Input &) { trs::rotate_z(*s.selected, -s.cfg.keySensitivity); }},

    {Mode::SCALE, GLFW_KEY_H, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1.0f - s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE, GLFW_KEY_L, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1.0f + s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE, GLFW_KEY_K, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1, 1.0f + s.cfg.keySensitivity, 1); }},
    {Mode::SCALE, GLFW_KEY_J, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1, 1.0f - s.cfg.keySensitivity, 1); }},
    {Mode::SCALE, GLFW_KEY_N, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1, 1, 1.0f + s.cfg.keySensitivity); }},
    {Mode::SCALE, GLFW_KEY_P, 0,
     [](EditorState &s, const Input &) { trs::scale(*s.selected, 1, 1, 1.0f - s.cfg.keySensitivity); }},
};

void editor::process_input(const Input &in, EditorState &state)
{
    for (const Binding &b : bindings)
    {
        if ((b.mode == state.mode || b.mode == Mode::ANY) && in.keys[b.key] && b.mods == in.mods)
            b.action(state, in);
    }
}
