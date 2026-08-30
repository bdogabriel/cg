#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "command.h"
#include "input.h"
#include "trs.h"

constexpr int MAX_UNDO = 32;

struct Editor
{
    Buffer buffer;
    TRS transforms[MAX_REFS] = {};
    Input *input = nullptr;

    Buffer undoBuffers[MAX_UNDO];
    TRS undoTransforms[MAX_UNDO][MAX_REFS];
    int undoHead = 0;
    int undoCount = 0;

    Ref selectedRef = 0;
    int faceCursor = 0;
    int selectedFaces[MAX_INDICES / 3] = {};
    int selectedFaceCount = 0;
    const Command *cmd = &cmd::none;
    Args args;
    float keySensitivity = 0.05f;
    float defaultMeshScale = 0.3f;
    bool wireframe = false;
    bool shouldQuit = false;
    char currentFile[256] = {};
};

namespace editor
{
void setup(Editor &e, Input &input);
void process_input(Editor &e);
void push_undo(Editor &e);
void pop_undo(Editor &e);
} // namespace editor

#endif
