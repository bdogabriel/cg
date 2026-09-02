#ifndef EDITOR_H
#define EDITOR_H

#include "meshbatch.h"
#include "trs.h"

constexpr int MAX_UNDO = 32;

enum class Target
{
    Object,
    Face
};

enum class AxisLock
{
    None,
    X,
    Y,
    Z
};

// TODO: decouple god struct
struct Editor
{
    MeshBatch meshBatch;
    TRS transforms[mesh::MAX_REFS] = {};

    MeshBatch undoBatches[MAX_UNDO];
    TRS undoTransforms[MAX_UNDO][mesh::MAX_REFS];
    int undoHead = 0;
    int undoCount = 0;

    Ref selectedRef = 0;
    int faceCursor = 0;
    int selectedFaces[face::MAX_FACES] = {};
    int selectedFaceCount = 0;
    float keySensitivity = 0.03f;
    float defaultMeshScale = 0.3f;
    bool wireframe = false;
    Target target = Target::Object;
    bool locked = false;
    Color bgColor = {0, 0, 0, 255};
    Color currentColor = {255, 255, 255, 255};
};

namespace editor
{
void push_undo(Editor &e);
void pop_undo(Editor &e);
} // namespace editor

#endif
