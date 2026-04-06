#include "editor.h"
#include "trs.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// TODO: review keybindings for more convinience:
// - hold n/p to cycle selection (might require key repeat timer)
// - review rotation to allow world rotation in any mode (it's how the user is able to see the model)
// TODO: review keybindings architecture
// TODO: review .obj and .mtl file logic (AI slop)

static void extrude_active_faces(EditorState &s, DrawBuffer &buf)
{
    if (s.selectedFaceCount > 0)
    {
        buf.extrude_faces(s.selectedRef, s.selectedFaces, s.selectedFaceCount);
    }
    else
    {
        buf.extrude_faces(s.selectedRef, &s.faceCursor, 1);
    }
}

static void transform_active_faces(EditorState &s, DrawBuffer &buf, Mat4 t)
{
    if (s.selectedFaceCount > 0)
    {
        buf.transform_faces(s.selectedRef, s.selectedFaces, s.selectedFaceCount, t);
    }
    else
    {
        buf.transform_faces(s.selectedRef, &s.faceCursor, 1, t);
    }
}

static void face_next(EditorState &s, DrawBuffer &buf, const Input &)
{
    int total = buf.commands[s.selectedRef].indicesCount / 3;
    s.faceCursor = (s.faceCursor + 1) % total;
}

static void face_prev(EditorState &s, DrawBuffer &buf, const Input &)
{
    int total = buf.commands[s.selectedRef].indicesCount / 3;
    s.faceCursor = (s.faceCursor - 1 + total) % total;
}

static void face_toggle_select(EditorState &s, DrawBuffer &, const Input &)
{
    bool found = false;
    for (int i = 0; i < s.selectedFaceCount; i++)
    {
        if (s.selectedFaces[i] == s.faceCursor)
        {
            s.selectedFaces[i] = s.selectedFaces[--s.selectedFaceCount];
            found = true;
            break;
        }
    }
    if (!found)
    {
        s.selectedFaces[s.selectedFaceCount++] = s.faceCursor;
    }
}

static const Binding bindings[] = {
    {Mode::ANY, GLFW_KEY_ESCAPE, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) {
        s.mode = Mode::NORMAL;
        s.selectedFaceCount = 0;
    }},

    {Mode::ANY, GLFW_KEY_W, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.wireframe = !s.wireframe; }},
    {Mode::ANY, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::ANY, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ANY, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::ANY, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},
    {Mode::ANY, GLFW_KEY_E, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::EXTRUDE_FACE; }},

    {Mode::TRANSLATE,   GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_X, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::ROTATE,      GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_X,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_Y,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_Z,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::SCALE,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_X,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_Y,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_Z,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SHEAR,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_X,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_Y,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_Z,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Z; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Z; }},
    {Mode::SCALE_FACE,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Z; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Z; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Z; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Z; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Z; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE,       GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},

    {Mode::TRANSLATE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},

    {Mode::ROTATE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},

    {Mode::SCALE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::SCALE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::SCALE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},

    {Mode::SHEAR,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},

    {Mode::ROTATE_FACE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},

    {Mode::SCALE_FACE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},

    {Mode::SHEAR_FACE,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},

    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},

    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},

    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},

    {Mode::TRANSLATE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity); }},
    {Mode::TRANSLATE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity); }},

    {Mode::ROTATE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateYNeg * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateY    * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateX    * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateXNeg * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateZ    * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = s.cfg.rotateZNeg * buf.transforms[s.selectedRef].r; }},
    {Mode::ROTATE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateXNeg; }},
    {Mode::ROTATE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateX;    }},
    {Mode::ROTATE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateYNeg; }},
    {Mode::ROTATE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateY;    }},
    {Mode::ROTATE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateZNeg; }},
    {Mode::ROTATE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { buf.transforms[s.selectedRef].r = buf.transforms[s.selectedRef].r * s.cfg.rotateZ;    }},

    {Mode::SCALE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f - s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f + s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f - s.cfg.keySensitivity, 1); }},
    {Mode::SCALE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f + s.cfg.keySensitivity, 1); }},
    {Mode::SCALE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f + s.cfg.keySensitivity); }},
    {Mode::SCALE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f - s.cfg.keySensitivity); }},
    {Mode::SCALE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f - s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f + s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f - s.cfg.keySensitivity, 1); }},
    {Mode::SCALE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f + s.cfg.keySensitivity, 1); }},
    {Mode::SCALE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f - s.cfg.keySensitivity); }},
    {Mode::SCALE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f + s.cfg.keySensitivity); }},

    {Mode::SHEAR_X, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, -s.cfg.keySensitivity, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0,  s.cfg.keySensitivity, 0, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, -s.cfg.keySensitivity, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0,  s.cfg.keySensitivity, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, 0, -s.cfg.keySensitivity); }},
    {Mode::SHEAR_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, 0,  s.cfg.keySensitivity); }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateXNeg); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateX);    }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateYNeg); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateY);    }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateZ);    }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateZNeg); }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateXNeg); }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateX);    }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateYNeg); }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateY);    }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateZNeg); }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.translateZ);    }},

    {Mode::ROTATE_FACE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateYNeg); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateY);    }},
    {Mode::ROTATE_FACE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateX);    }},
    {Mode::ROTATE_FACE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateXNeg); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateZ);    }},
    {Mode::ROTATE_FACE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateZNeg); }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateXNeg); }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateX);    }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateYNeg); }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateY);    }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateZNeg); }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.rotateZ);    }},

    {Mode::SCALE_FACE,   GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleXNeg); }},
    {Mode::SCALE_FACE,   GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleX);    }},
    {Mode::SCALE_FACE,   GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleYNeg); }},
    {Mode::SCALE_FACE,   GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleY);    }},
    {Mode::SCALE_FACE,   GLFW_KEY_U, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleZ);    }},
    {Mode::SCALE_FACE,   GLFW_KEY_D, GLFW_MOD_CONTROL, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleZNeg); }},
    {Mode::SCALE_FACE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleXNeg); }},
    {Mode::SCALE_FACE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleX);    }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleYNeg); }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleY);    }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleZNeg); }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.scaleZ);    }},

    {Mode::SHEAR_FACE_X, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearXYNeg); }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearXY);    }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearXZNeg); }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearXZ);    }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearYXNeg); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearYX);    }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearYZNeg); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearYZ);    }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_H, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearZXNeg); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_L, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearZX);    }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_J, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearZYNeg); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_K, 0, false, true, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, s.cfg.shearZY);    }},

    {Mode::TRANSLATE_FACE,         GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE,         GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_X,       GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_X,       GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_Y,       GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_Y,       GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_Z,       GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_Z,       GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE,            GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE,            GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_X,          GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_X,          GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_Y,          GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_Y,          GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_Z,          GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_Z,          GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE,             GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE,             GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_X,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_X,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_Y,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_Y,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_Z,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_Z,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE,             GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE,             GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_X,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_X,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_Y,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_Y,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_Z,           GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_Z,           GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_N, 0, true, false, face_next}, {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_N, 0, true, false, face_next}, {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_N, 0, true, false, face_next}, {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_P, 0, true, false, face_prev},

    {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_SPACE, 0, true, false, face_toggle_select},

    // EXTRUDE_FACE mode entry from any mode

    // EXTRUDE_FACE actions: E extrudes the selected faces once with a small fixed offset
    {Mode::EXTRUDE_FACE, GLFW_KEY_SPACE, 0, true, true, [](EditorState &s, DrawBuffer &buf, const Input &) { extrude_active_faces(s, buf); }},

    {Mode::EXTRUDE_FACE, GLFW_KEY_N, 0, true, false, face_next},
    {Mode::EXTRUDE_FACE, GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::EXTRUDE_FACE, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::EXTRUDE_FACE_VISUAL; }},
    {Mode::EXTRUDE_FACE, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::NORMAL; }},
    {Mode::EXTRUDE_FACE, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::EXTRUDE_FACE, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::EXTRUDE_FACE, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::EXTRUDE_FACE, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},

    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_N, 0, true, false, face_next},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_P, 0, true, false, face_prev},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_SPACE, 0, true, false, face_toggle_select},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_V, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::EXTRUDE_FACE; }},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_F, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::NORMAL; }},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_T, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_R, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_S, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::EXTRUDE_FACE_VISUAL, GLFW_KEY_A, 0, true, false, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
};

static std::string mtl_path(const std::string &obj_path)
{
    size_t dot = obj_path.rfind('.');
    if (dot == std::string::npos)
    {
        return obj_path + ".mtl";
    }
    return obj_path.substr(0, dot) + ".mtl";
}

static std::string path_basename(const std::string &path)
{
    size_t slash = path.rfind('/');
    if (slash == std::string::npos)
    {
        return path;
    }
    return path.substr(slash + 1);
}

static void obj_export(const DrawBuffer &buf, Ref obj, const std::string &path)
{
    const DrawCommand &cmd = buf.commands[obj];

    int maxVtx = -1;
    for (unsigned int i = 0; i < cmd.indicesCount; i++)
    {
        int idx = (int)buf.indices[cmd.indexOffset + i];
        if (idx > maxVtx)
        {
            maxVtx = idx;
        }
    }
    int vtxCount = maxVtx + 1;
    int faceCount = cmd.indicesCount / 3;
    int faceOffset = buf.faceOffsets[obj];

    std::string mpath = mtl_path(path);
    std::ofstream mtl(mpath);
    if (!mtl)
    {
        printf("E: cannot open %s for writing\n", mpath.c_str());
        return;
    }
    for (int i = 0; i < faceCount; i++)
    {
        const Color &col = buf.faceColors[faceOffset + i];
        mtl << "newmtl mat_" << i << "\n";
        mtl << "Kd " << col.r << " " << col.g << " " << col.b << "\n";
        mtl << "d " << col.a << "\n\n";
    }

    std::ofstream f(path);
    if (!f)
    {
        printf("E: cannot open %s for writing\n", path.c_str());
        return;
    }

    f << "mtllib " << path_basename(mpath) << "\no model\n";
    for (int i = 0; i < vtxCount; i++)
    {
        Vec4 w = buf.models[obj] * buf.vertices[cmd.vertexOffset + i];
        f << "v " << w.x << " " << w.y << " " << w.z << "\n";
    }
    for (unsigned int i = 0; i < cmd.indicesCount; i += 3)
    {
        f << "usemtl mat_" << i / 3 << "\n";
        f << "f " << buf.indices[cmd.indexOffset + i + 0] + 1 << " " << buf.indices[cmd.indexOffset + i + 1] + 1 << " "
          << buf.indices[cmd.indexOffset + i + 2] + 1 << "\n";
    }

    printf("saved %s\n", path.c_str());
}

static bool obj_import(DrawBuffer &buf, EditorState &state, const std::string &path)
{
    std::ifstream f(path);
    if (!f)
    {
        printf("E: cannot open %s\n", path.c_str());
        return false;
    }

    Color matColors[MAX_INDICES / 3] = {};
    Color defaultColor = {1.0f, 0.6f, 0.2f, 1.0f};

    Vec4 loadedVerts[MAX_VERTICES];
    int loadedVtxCount = 0;
    unsigned int loadedIndices[MAX_INDICES];
    int loadedIdxCount = 0;
    Color loadedColors[MAX_INDICES / 3];
    int loadedFaceCount = 0;

    Color currentColor = defaultColor;

    std::string line;
    while (std::getline(f, line))
    {
        if (line.size() > 7 && line[0] == 'm' && line.substr(0, 7) == "mtllib ")
        {
            std::string mpath = mtl_path(path);
            std::ifstream mf(mpath);
            if (mf)
            {
                int idx = -1;
                std::string mline;
                while (std::getline(mf, mline))
                {
                    if (mline.size() > 7 && mline.substr(0, 7) == "newmtl ")
                    {
                        sscanf(mline.c_str() + 7, "mat_%d", &idx);
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            matColors[idx] = defaultColor;
                        }
                    }
                    else if (mline.size() > 3 && mline[0] == 'K' && mline[1] == 'd' && mline[2] == ' ')
                    {
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            sscanf(mline.c_str() + 3, "%f %f %f", &matColors[idx].r, &matColors[idx].g,
                                   &matColors[idx].b);
                        }
                    }
                    else if (mline.size() > 2 && mline[0] == 'd' && mline[1] == ' ')
                    {
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            sscanf(mline.c_str() + 2, "%f", &matColors[idx].a);
                        }
                    }
                }
            }
        }
        else if (line.size() > 7 && line.substr(0, 7) == "usemtl ")
        {
            int idx = -1;
            sscanf(line.c_str() + 7, "mat_%d", &idx);
            if (idx >= 0 && idx < MAX_INDICES / 3)
            {
                currentColor = matColors[idx];
            }
        }
        else if (line.size() >= 2 && line[0] == 'v' && line[1] == ' ')
        {
            if (loadedVtxCount >= MAX_VERTICES)
            {
                printf("E: too many vertices in %s\n", path.c_str());
                return false;
            }
            float x, y, z;
            sscanf(line.c_str() + 2, "%f %f %f", &x, &y, &z);
            loadedVerts[loadedVtxCount++] = {x, y, z, 1.0f};
        }
        else if (line.size() >= 2 && line[0] == 'f' && line[1] == ' ')
        {
            if (loadedIdxCount + 3 > MAX_INDICES)
            {
                printf("E: too many faces in %s\n", path.c_str());
                return false;
            }
            unsigned int a, b, c;
            sscanf(line.c_str() + 2, "%u %u %u", &a, &b, &c);
            loadedIndices[loadedIdxCount++] = a - 1;
            loadedIndices[loadedIdxCount++] = b - 1;
            loadedIndices[loadedIdxCount++] = c - 1;
            loadedColors[loadedFaceCount++] = currentColor;
        }
    }

    buf.vtxCount = 0;
    buf.idxCount = 0;
    buf.objCount = 1;

    Geometry geo = {loadedVerts, loadedVtxCount, loadedIndices, loadedIdxCount, loadedColors, loadedFaceCount};
    Ref newRef = buf.add(geo, TRS{});
    buf.update();

    state.selectedRef = newRef;
    state.faceCursor = 0;
    state.selectedFaceCount = 0;
    state.mode = Mode::NORMAL;

    printf("loaded %s\n", path.c_str());
    return true;
}

static void execute_command(EditorState &state, DrawBuffer &buf, const std::string &cmd)
{
    if (cmd == "q")
    {
        state.shouldQuit = true;
    }
    else if (cmd == "w")
    {
        if (state.currentFile.empty())
        {
            state.currentFile = "model.obj";
        }
        obj_export(buf, state.selectedRef, state.currentFile);
    }
    else if (cmd.size() > 2 && cmd[0] == 'w' && cmd[1] == ' ')
    {
        state.currentFile = cmd.substr(2);
        obj_export(buf, state.selectedRef, state.currentFile);
    }
    else if (cmd.size() > 2 && cmd[0] == 'e' && cmd[1] == ' ')
    {
        std::string path = cmd.substr(2);
        if (obj_import(buf, state, path))
        {
            state.currentFile = path;
        }
    }
    else
    {
        printf("E: unknown command: %s\n", cmd.c_str());
    }
}

static const char *mode_name(Mode m)
{
    switch (m)
    {
    case Mode::ANY:
        return "ANY";
    case Mode::NORMAL:
        return "NORMAL";
    case Mode::TRANSLATE:
        return "TRANSLATE";
    case Mode::TRANSLATE_X:
        return "TRANSLATE_X";
    case Mode::TRANSLATE_Y:
        return "TRANSLATE_Y";
    case Mode::TRANSLATE_Z:
        return "TRANSLATE_Z";
    case Mode::ROTATE:
        return "ROTATE";
    case Mode::ROTATE_X:
        return "ROTATE_X";
    case Mode::ROTATE_Y:
        return "ROTATE_Y";
    case Mode::ROTATE_Z:
        return "ROTATE_Z";
    case Mode::SCALE:
        return "SCALE";
    case Mode::SCALE_X:
        return "SCALE_X";
    case Mode::SCALE_Y:
        return "SCALE_Y";
    case Mode::SCALE_Z:
        return "SCALE_Z";
    case Mode::SHEAR:
        return "SHEAR";
    case Mode::SHEAR_X:
        return "SHEAR_X";
    case Mode::SHEAR_Y:
        return "SHEAR_Y";
    case Mode::SHEAR_Z:
        return "SHEAR_Z";
    case Mode::TRANSLATE_FACE:
        return "TRANSLATE_FACE";
    case Mode::TRANSLATE_FACE_X:
        return "TRANSLATE_FACE_X";
    case Mode::TRANSLATE_FACE_Y:
        return "TRANSLATE_FACE_Y";
    case Mode::TRANSLATE_FACE_Z:
        return "TRANSLATE_FACE_Z";
    case Mode::ROTATE_FACE:
        return "ROTATE_FACE";
    case Mode::ROTATE_FACE_X:
        return "ROTATE_FACE_X";
    case Mode::ROTATE_FACE_Y:
        return "ROTATE_FACE_Y";
    case Mode::ROTATE_FACE_Z:
        return "ROTATE_FACE_Z";
    case Mode::SCALE_FACE:
        return "SCALE_FACE";
    case Mode::SCALE_FACE_X:
        return "SCALE_FACE_X";
    case Mode::SCALE_FACE_Y:
        return "SCALE_FACE_Y";
    case Mode::SCALE_FACE_Z:
        return "SCALE_FACE_Z";
    case Mode::SHEAR_FACE:
        return "SHEAR_FACE";
    case Mode::SHEAR_FACE_X:
        return "SHEAR_FACE_X";
    case Mode::SHEAR_FACE_Y:
        return "SHEAR_FACE_Y";
    case Mode::SHEAR_FACE_Z:
        return "SHEAR_FACE_Z";
    case Mode::TRANSLATE_FACE_VISUAL:
        return "TRANSLATE_FACE_VISUAL";
    case Mode::TRANSLATE_FACE_X_VISUAL:
        return "TRANSLATE_FACE_X_VISUAL";
    case Mode::TRANSLATE_FACE_Y_VISUAL:
        return "TRANSLATE_FACE_Y_VISUAL";
    case Mode::TRANSLATE_FACE_Z_VISUAL:
        return "TRANSLATE_FACE_Z_VISUAL";
    case Mode::ROTATE_FACE_VISUAL:
        return "ROTATE_FACE_VISUAL";
    case Mode::ROTATE_FACE_X_VISUAL:
        return "ROTATE_FACE_X_VISUAL";
    case Mode::ROTATE_FACE_Y_VISUAL:
        return "ROTATE_FACE_Y_VISUAL";
    case Mode::ROTATE_FACE_Z_VISUAL:
        return "ROTATE_FACE_Z_VISUAL";
    case Mode::SCALE_FACE_VISUAL:
        return "SCALE_FACE_VISUAL";
    case Mode::SCALE_FACE_X_VISUAL:
        return "SCALE_FACE_X_VISUAL";
    case Mode::SCALE_FACE_Y_VISUAL:
        return "SCALE_FACE_Y_VISUAL";
    case Mode::SCALE_FACE_Z_VISUAL:
        return "SCALE_FACE_Z_VISUAL";
    case Mode::SHEAR_FACE_VISUAL:
        return "SHEAR_FACE_VISUAL";
    case Mode::SHEAR_FACE_X_VISUAL:
        return "SHEAR_FACE_X_VISUAL";
    case Mode::SHEAR_FACE_Y_VISUAL:
        return "SHEAR_FACE_Y_VISUAL";
    case Mode::SHEAR_FACE_Z_VISUAL:
        return "SHEAR_FACE_Z_VISUAL";
    case Mode::EXTRUDE_FACE:
        return "EXTRUDE_FACE";
    case Mode::EXTRUDE_FACE_VISUAL:
        return "EXTRUDE_FACE_VISUAL";
    }
    return "?";
}

void editor::build_highlights(const EditorState &state, DrawBuffer &buf, Ref obj, DrawBuffer &highlight, Mat4 model)
{
    if (state.mode < Mode::TRANSLATE_FACE)
    {
        return;
    }
    buf.add_face_highlights(highlight, obj, state.faceCursor, {1.0f, 0.8f, 0.0f, 1}, model);
    for (int i = 0; i < state.selectedFaceCount; i++)
    {
        buf.add_face_highlights(highlight, obj, state.selectedFaces[i], {1.0f, 0.4f, 0.0f, 1}, model);
    }
}

void editor::process_input(const Input &in, EditorState &state, DrawBuffer &buf, UndoStack &undo)
{
    if (in.keys[GLFW_KEY_SEMICOLON] == KeyState::JustPressed && (in.mods & GLFW_MOD_SHIFT))
    {
        printf(":");
        fflush(stdout);
        std::string cmd;
        std::getline(std::cin, cmd);
        execute_command(state, buf, cmd);
        return;
    }

    if (in.keys[GLFW_KEY_U] == KeyState::Released && in.mods == 0)
    {
        if (undo.pop(buf))
        {
            buf.update();
        }
    }

    Mode mode = state.mode;
    for (const Binding &b : bindings)
    {
        bool triggered = b.oneShot ? in.keys[b.key] == KeyState::JustPressed
                                   : (in.keys[b.key] == KeyState::Down || in.keys[b.key] == KeyState::JustPressed);
        if ((b.mode == mode || b.mode == Mode::ANY) && triggered && b.mods == in.mods)
        {
            if (b.pushesUndo && in.keys[b.key] == KeyState::JustPressed)
            {
                undo.push(buf);
            }
            b.action(state, buf, in);
        }
    }
    if (state.mode != mode)
    {
        printf("%s\n", mode_name(state.mode));
    }
}
