#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "camera.h"
#include "keyboard.h"
#include "mesh_batch.h"
#include "model_texture_cache.h"
#include "param.h"
#include "prompt.h"
#include "skybox.h"
#include "window.h"

constexpr int MAX_UNDO = 100;

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

struct Editor
{
    MeshBatch meshBatch;
    ModelTextureCache modelTextureCache;
    Buffer transforms; // TRS; indexed by slot
    Camera camera;
    Skybox skybox;

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
    Target target = Target::Object;
    bool locked = false;
    Color bgColor = {0, 0, 0, 255};
    Color currentColor = {255, 255, 255, 255};

    CommandId cmd = CommandId::camera;
    Args args;
    Prompt prompt;
    Keyboard keyboard;
    Window window;
    char currentFile[256] = {};
};

namespace editor
{
void push_undo(Editor &e);
void pop_undo(Editor &e);

inline bool is_cmd_exec(const Editor &e)
{
    return e.cmd == CommandId::execute;
}
} // namespace editor

#endif
