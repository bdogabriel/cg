#include "editor.h"
#include "trs.h"
#include <GLFW/glfw3.h>
#include <cstdio>

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
    {Mode::ANY, GLFW_KEY_ESCAPE, 0, true, [](EditorState &s, DrawBuffer &, const Input &) {
        s.mode = Mode::NORMAL;
        s.selectedFaceCount = 0;
    }},

    {Mode::ANY, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::ANY, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ANY, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::ANY, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},

    {Mode::TRANSLATE,   GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_X, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::ROTATE,      GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_X,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_Y,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_Z,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::SCALE,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_X,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_Y,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_Z,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SHEAR,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_X,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_Y,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_Z,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Z; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Z; }},
    {Mode::SCALE_FACE,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Z; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Z; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::TRANSLATE_Z; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::ROTATE_Z; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SCALE_Z; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_F, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.selectedFaceCount = 0; s.mode = Mode::SHEAR_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE,       GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_V, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE,      GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_X,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_Y,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::ROTATE_FACE_Z,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE,       GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Y,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Z,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE,       GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_X,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_Y,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SHEAR_FACE_Z,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,      GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL,    GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,       GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,       GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_T, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_R, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_S, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL,     GLFW_KEY_E, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},

    {Mode::TRANSLATE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},
    {Mode::TRANSLATE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Z; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_X; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_Y; }},
    {Mode::TRANSLATE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE; }},

    {Mode::ROTATE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},
    {Mode::ROTATE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Z; }},
    {Mode::ROTATE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_X; }},
    {Mode::ROTATE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_Y; }},
    {Mode::ROTATE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE; }},

    {Mode::SCALE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::SCALE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},
    {Mode::SCALE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Z; }},
    {Mode::SCALE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_X; }},
    {Mode::SCALE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_Y; }},
    {Mode::SCALE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE; }},

    {Mode::SHEAR,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},
    {Mode::SHEAR_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Z; }},
    {Mode::SHEAR_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_X; }},
    {Mode::SHEAR_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_Y; }},
    {Mode::SHEAR_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR; }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y; }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE; }},

    {Mode::ROTATE_FACE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y; }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE; }},

    {Mode::SCALE_FACE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y; }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE; }},

    {Mode::SHEAR_FACE,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y; }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE; }},

    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},
    {Mode::TRANSLATE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Z_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_X_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_Y_VISUAL; }},
    {Mode::TRANSLATE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::TRANSLATE_FACE_VISUAL; }},

    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},
    {Mode::ROTATE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Z_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_X_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_Y_VISUAL; }},
    {Mode::ROTATE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::ROTATE_FACE_VISUAL; }},

    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_VISUAL,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},
    {Mode::SCALE_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Z_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_X_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_Y_VISUAL; }},
    {Mode::SCALE_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SCALE_FACE_VISUAL; }},

    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_VISUAL,   GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_X_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},
    {Mode::SHEAR_FACE_Y_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Z_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_X, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_X_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_Y, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_Y_VISUAL; }},
    {Mode::SHEAR_FACE_Z_VISUAL, GLFW_KEY_Z, 0, true, [](EditorState &s, DrawBuffer &, const Input &) { s.mode = Mode::SHEAR_FACE_VISUAL; }},

    {Mode::TRANSLATE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity); }},
    {Mode::TRANSLATE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0); }},
    {Mode::TRANSLATE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0); }},
    {Mode::TRANSLATE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity); }},
    {Mode::TRANSLATE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::translate_local(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity); }},

    {Mode::ROTATE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_y(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_y(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},
    {Mode::ROTATE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_x(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},
    {Mode::ROTATE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_x(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_z(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},
    {Mode::ROTATE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_z(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_x_local(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_x_local(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},
    {Mode::ROTATE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_y_local(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_y_local(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},
    {Mode::ROTATE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_z_local(buf.transforms[s.selectedRef], -s.cfg.keySensitivity); }},
    {Mode::ROTATE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::rotate_z_local(buf.transforms[s.selectedRef],  s.cfg.keySensitivity); }},

    {Mode::SCALE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f - s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f + s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f - s.cfg.keySensitivity, 1); }},
    {Mode::SCALE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f + s.cfg.keySensitivity, 1); }},
    {Mode::SCALE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f + s.cfg.keySensitivity); }},
    {Mode::SCALE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f - s.cfg.keySensitivity); }},
    {Mode::SCALE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f - s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1.0f + s.cfg.keySensitivity, 1, 1); }},
    {Mode::SCALE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f - s.cfg.keySensitivity, 1); }},
    {Mode::SCALE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1.0f + s.cfg.keySensitivity, 1); }},
    {Mode::SCALE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f - s.cfg.keySensitivity); }},
    {Mode::SCALE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::scale(buf.transforms[s.selectedRef], 1, 1, 1.0f + s.cfg.keySensitivity); }},

    {Mode::SHEAR_X, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], -s.cfg.keySensitivity, 0, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef],  s.cfg.keySensitivity, 0, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, -s.cfg.keySensitivity, 0, 0, 0, 0); }},
    {Mode::SHEAR_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0,  s.cfg.keySensitivity, 0, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, -s.cfg.keySensitivity, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0,  s.cfg.keySensitivity, 0, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, -s.cfg.keySensitivity, 0, 0); }},
    {Mode::SHEAR_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0,  s.cfg.keySensitivity, 0, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, -s.cfg.keySensitivity, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0,  s.cfg.keySensitivity, 0); }},
    {Mode::SHEAR_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, 0, -s.cfg.keySensitivity); }},
    {Mode::SHEAR_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { trs::shear(buf.transforms[s.selectedRef], 0, 0, 0, 0, 0,  s.cfg.keySensitivity); }},

    {Mode::TRANSLATE_FACE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(-s.cfg.keySensitivity, 0, 0)); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation( s.cfg.keySensitivity, 0, 0)); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, -s.cfg.keySensitivity, 0)); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0,  s.cfg.keySensitivity, 0)); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, 0,  s.cfg.keySensitivity)); }},
    {Mode::TRANSLATE_FACE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, 0, -s.cfg.keySensitivity)); }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(-s.cfg.keySensitivity, 0, 0)); }},
    {Mode::TRANSLATE_FACE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation( s.cfg.keySensitivity, 0, 0)); }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, -s.cfg.keySensitivity, 0)); }},
    {Mode::TRANSLATE_FACE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0,  s.cfg.keySensitivity, 0)); }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, 0, -s.cfg.keySensitivity)); }},
    {Mode::TRANSLATE_FACE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::translation(0, 0,  s.cfg.keySensitivity)); }},

    {Mode::ROTATE_FACE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_y(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_y( s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_x( s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_x(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_z( s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_z(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_x(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_x( s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_y(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_y( s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_z(-s.cfg.keySensitivity)); }},
    {Mode::ROTATE_FACE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::rotation_z( s.cfg.keySensitivity)); }},

    {Mode::SCALE_FACE,   GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1.0f - s.cfg.keySensitivity, 1, 1)); }},
    {Mode::SCALE_FACE,   GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1.0f + s.cfg.keySensitivity, 1, 1)); }},
    {Mode::SCALE_FACE,   GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1.0f - s.cfg.keySensitivity, 1)); }},
    {Mode::SCALE_FACE,   GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1.0f + s.cfg.keySensitivity, 1)); }},
    {Mode::SCALE_FACE,   GLFW_KEY_U, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1, 1.0f + s.cfg.keySensitivity)); }},
    {Mode::SCALE_FACE,   GLFW_KEY_D, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1, 1.0f - s.cfg.keySensitivity)); }},
    {Mode::SCALE_FACE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1.0f - s.cfg.keySensitivity, 1, 1)); }},
    {Mode::SCALE_FACE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1.0f + s.cfg.keySensitivity, 1, 1)); }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1.0f - s.cfg.keySensitivity, 1)); }},
    {Mode::SCALE_FACE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1.0f + s.cfg.keySensitivity, 1)); }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1, 1.0f - s.cfg.keySensitivity)); }},
    {Mode::SCALE_FACE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::scaling(1, 1, 1.0f + s.cfg.keySensitivity)); }},

    {Mode::SHEAR_FACE_X, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(-s.cfg.keySensitivity, 0, 0, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing( s.cfg.keySensitivity, 0, 0, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, -s.cfg.keySensitivity, 0, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_X, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0,  s.cfg.keySensitivity, 0, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, -s.cfg.keySensitivity, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0,  s.cfg.keySensitivity, 0, 0, 0)); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0, -s.cfg.keySensitivity, 0, 0)); }},
    {Mode::SHEAR_FACE_Y, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0,  s.cfg.keySensitivity, 0, 0)); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_H, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0, 0, -s.cfg.keySensitivity, 0)); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_L, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0, 0,  s.cfg.keySensitivity, 0)); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_J, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0, 0, 0, -s.cfg.keySensitivity)); }},
    {Mode::SHEAR_FACE_Z, GLFW_KEY_K, 0, false, [](EditorState &s, DrawBuffer &buf, const Input &) { transform_active_faces(s, buf, trs::shearing(0, 0, 0, 0, 0,  s.cfg.keySensitivity)); }},

    {Mode::TRANSLATE_FACE,         GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE,         GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_X,       GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_X,       GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_Y,       GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_Y,       GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_Z,       GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_Z,       GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE,            GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE,            GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_X,          GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_X,          GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_Y,          GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_Y,          GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_Z,          GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_Z,          GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE,             GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE,             GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_X,           GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_X,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_Y,           GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_Y,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_Z,           GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_Z,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE,             GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE,             GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_X,           GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_X,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_Y,           GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_Y,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_Z,           GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_Z,           GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_P, 0, true, face_prev},
    {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_N, 0, true, face_next}, {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_P, 0, true, face_prev},
    {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_N, 0, true, face_next}, {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_P, 0, true, face_prev},
    {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_P, 0, true, face_prev},
    {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_N, 0, true, face_next}, {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_P, 0, true, face_prev},

    {Mode::TRANSLATE_FACE_VISUAL,  GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::TRANSLATE_FACE_X_VISUAL,GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::TRANSLATE_FACE_Y_VISUAL,GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::TRANSLATE_FACE_Z_VISUAL,GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::ROTATE_FACE_VISUAL,     GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::ROTATE_FACE_X_VISUAL,   GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::ROTATE_FACE_Y_VISUAL,   GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::ROTATE_FACE_Z_VISUAL,   GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SCALE_FACE_VISUAL,      GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SCALE_FACE_X_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SCALE_FACE_Y_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SCALE_FACE_Z_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SHEAR_FACE_VISUAL,      GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SHEAR_FACE_X_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SHEAR_FACE_Y_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
    {Mode::SHEAR_FACE_Z_VISUAL,    GLFW_KEY_SPACE, 0, true, face_toggle_select},
};

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
    }
    return "?";
}

void editor::process_input(const Input &in, EditorState &state, DrawBuffer &buf)
{
    Mode mode = state.mode;
    for (const Binding &b : bindings)
    {
        bool triggered = in.keys[b.key] == (b.oneShot ? KeyState::Released : KeyState::Down);
        if ((b.mode == mode || b.mode == Mode::ANY) && triggered && b.mods == in.mods)
        {
            b.action(state, buf, in);
        }
    }
    if (state.mode != mode)
    {
        printf("mode: %s\n", mode_name(state.mode));
    }
}
