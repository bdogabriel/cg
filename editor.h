#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
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
    TRANSLATE_FACE,
    TRANSLATE_FACE_X,
    TRANSLATE_FACE_Y,
    TRANSLATE_FACE_Z,
    ROTATE_FACE,
    ROTATE_FACE_X,
    ROTATE_FACE_Y,
    ROTATE_FACE_Z,
    SCALE_FACE,
    SCALE_FACE_X,
    SCALE_FACE_Y,
    SCALE_FACE_Z,
    SHEAR_FACE,
    SHEAR_FACE_X,
    SHEAR_FACE_Y,
    SHEAR_FACE_Z,
    TRANSLATE_FACE_VISUAL,
    TRANSLATE_FACE_X_VISUAL,
    TRANSLATE_FACE_Y_VISUAL,
    TRANSLATE_FACE_Z_VISUAL,
    ROTATE_FACE_VISUAL,
    ROTATE_FACE_X_VISUAL,
    ROTATE_FACE_Y_VISUAL,
    ROTATE_FACE_Z_VISUAL,
    SCALE_FACE_VISUAL,
    SCALE_FACE_X_VISUAL,
    SCALE_FACE_Y_VISUAL,
    SCALE_FACE_Z_VISUAL,
    SHEAR_FACE_VISUAL,
    SHEAR_FACE_X_VISUAL,
    SHEAR_FACE_Y_VISUAL,
    SHEAR_FACE_Z_VISUAL,
};

struct EditorConfig
{
    float keySensitivity = 0.05f;
};

struct EditorState
{
    EditorConfig cfg;
    Mode mode = Mode::NORMAL;
    Ref selectedRef = 0;
    int faceCursor = 0;
    int selectedFaces[MAX_INDICES / 3] = {};
    int selectedFaceCount = 0;
};

using ActionFn = void (*)(EditorState &, DrawBuffer &, const Input &);

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
void process_input(const Input &input, EditorState &state, DrawBuffer &buf);
}

#endif
