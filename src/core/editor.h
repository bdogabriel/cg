#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "camera.h"
#include "keyboard.h"
#include "mesh_batch.h"
#include "model_texture_cache.h"
#include "param.h"
#include "prompt.h"
#include "renderer.h"
#include "window.h"

struct Command;

constexpr int MAX_UNDO = 100;

enum class Target
{
    OBJECT,
    FACE
};

enum class AxisLock
{
    NONE,
    X,
    Y,
    Z
};

struct Editor
{
    MeshBatch meshBatch;
    ModelTextureCache modelTextureCache;
    Buffer transforms; // TRS; indexed by slot
    Camera camera;
    render::TextureHandle skyboxTexture = 0;

    MeshBatch undoMeshBatches[MAX_UNDO];
    Buffer undoTransforms[MAX_UNDO]; // TRS; indexed by slot
    int undoHead = 0;
    int undoCount = 0;

    Ref selectedRef = 0;
    int faceCursor = 0;
    Buffer selectedFaces; // int
    int selectedFaceCount = 0;
    float keySensitivity = 0.03f;
    float defaultMeshScale = 0.3f;
    bool wireframe = false;
    Target target = Target::OBJECT;
    bool locked = false;
    Color bgColor = {0, 0, 0, 255};
    Color currentColor = {255, 255, 255, 255};

    const Command *cmd = nullptr;
    Args args;
    Prompt prompt;
    Keyboard keyboard;
    Window window;
    char currentFile[256] = {};
    char skyboxPath[256] = {};
};

namespace editor
{
void push_undo(Editor &e);
void pop_undo(Editor &e);
} // namespace editor

#endif
