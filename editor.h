#ifndef EDITOR_H
#define EDITOR_H

#include "input.h"
#include "trs.h"

enum class Mode
{
    ANY,
    NORMAL,
    TRANSLATE,
    TRANSLATE_X,
    TRANSLATE_Y,
    TRANSLATE_Z,
    ROTATE,
    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z,
    SCALE,
    SCALE_X,
    SCALE_Y,
    SCALE_Z,
    SHEAR,
    SHEAR_X,
    SHEAR_Y,
    SHEAR_Z,
};

struct EditorConfig
{
    float keySensitivity = 0.025f;
};

struct EditorState
{
    Mode mode = Mode::NORMAL;
    TRS *selected = nullptr;
    EditorConfig cfg;
};

using ActionFn = void (*)(EditorState &, const Input &);

struct Binding
{
    Mode mode;
    int key;
    int mods;
    bool oneShot;
    ActionFn action;
};

namespace editor
{
void process_input(const Input &input, EditorState &state);
}

#endif
