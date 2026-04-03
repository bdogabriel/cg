#ifndef EDITOR_H
#define EDITOR_H

#include "input.h"
#include "trs.h"

enum class Mode
{
    ANY,
    TRANSLATE_LOCAL,
    TRANSLATE_WORLD,
    ROTATE_LOCAL,
    ROTATE_WORLD,
    SCALE,
};

struct EditorConfig
{
    float keySensitivity = 0.025f;
    float mouseSensitivity = 0.005f;
    float scrollSensitivity = 0.1f;
};

struct EditorState
{
    Mode mode = Mode::ROTATE_LOCAL;
    TRS *selected = nullptr;
    EditorConfig cfg;
};

using ActionFn = void (*)(EditorState &, const Input &);

struct Binding
{
    Mode mode;
    int key;
    int mods;
    ActionFn action;
};

namespace editor
{
void process_input(const Input &input, EditorState &state);
} // namespace editor

#endif // EDITOR_H
