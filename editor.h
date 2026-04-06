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
    EXTRUDE_FACE,
    EXTRUDE_FACE_VISUAL,
};

struct EditorConfig
{
    float keySensitivity = 0.05f;

    Mat4 rotateX = trs::rotation_x(keySensitivity);
    Mat4 rotateXNeg = trs::rotation_x(-keySensitivity);
    Mat4 rotateY = trs::rotation_y(keySensitivity);
    Mat4 rotateYNeg = trs::rotation_y(-keySensitivity);
    Mat4 rotateZ = trs::rotation_z(keySensitivity);
    Mat4 rotateZNeg = trs::rotation_z(-keySensitivity);

    Mat4 translateX = trs::translation(keySensitivity, 0, 0);
    Mat4 translateXNeg = trs::translation(-keySensitivity, 0, 0);
    Mat4 translateY = trs::translation(0, keySensitivity, 0);
    Mat4 translateYNeg = trs::translation(0, -keySensitivity, 0);
    Mat4 translateZ = trs::translation(0, 0, keySensitivity);
    Mat4 translateZNeg = trs::translation(0, 0, -keySensitivity);

    Mat4 scaleX = trs::scaling(1 + keySensitivity, 1, 1);
    Mat4 scaleXNeg = trs::scaling(1 - keySensitivity, 1, 1);
    Mat4 scaleY = trs::scaling(1, 1 + keySensitivity, 1);
    Mat4 scaleYNeg = trs::scaling(1, 1 - keySensitivity, 1);
    Mat4 scaleZ = trs::scaling(1, 1, 1 + keySensitivity);
    Mat4 scaleZNeg = trs::scaling(1, 1, 1 - keySensitivity);

    Mat4 shearXY = trs::shearing(keySensitivity, 0, 0, 0, 0, 0);
    Mat4 shearXYNeg = trs::shearing(-keySensitivity, 0, 0, 0, 0, 0);
    Mat4 shearXZ = trs::shearing(0, keySensitivity, 0, 0, 0, 0);
    Mat4 shearXZNeg = trs::shearing(0, -keySensitivity, 0, 0, 0, 0);
    Mat4 shearYX = trs::shearing(0, 0, keySensitivity, 0, 0, 0);
    Mat4 shearYXNeg = trs::shearing(0, 0, -keySensitivity, 0, 0, 0);
    Mat4 shearYZ = trs::shearing(0, 0, 0, keySensitivity, 0, 0);
    Mat4 shearYZNeg = trs::shearing(0, 0, 0, -keySensitivity, 0, 0);
    Mat4 shearZX = trs::shearing(0, 0, 0, 0, keySensitivity, 0);
    Mat4 shearZXNeg = trs::shearing(0, 0, 0, 0, -keySensitivity, 0);
    Mat4 shearZY = trs::shearing(0, 0, 0, 0, 0, keySensitivity);
    Mat4 shearZYNeg = trs::shearing(0, 0, 0, 0, 0, -keySensitivity);
};

struct EditorState
{
    EditorConfig cfg;
    Mode mode = Mode::NORMAL;
    Ref selectedRef = 0;
    int faceCursor = 0;
    int selectedFaces[MAX_INDICES / 3] = {};
    int selectedFaceCount = 0;
    bool wireframe = false;
};

using ActionFn = void (*)(EditorState &, DrawBuffer &, const Input &);

struct Binding
{
    Mode mode;
    int key;
    int mods;
    bool oneShot;
    bool pushesUndo;
    ActionFn action;
};

namespace editor
{
void process_input(const Input &input, EditorState &state, DrawBuffer &buf, UndoStack &undo);
}

#endif
