#ifndef BUFFER_H
#define BUFFER_H

#include "color.h"
#include "mat4.h"
#include "mesh.h"
#include <GL/glew.h>

// TODO: review architecture and separate logic into more files
// TODO: cycle faces sorted by distance to the camera (when camera is implemented)
// TODO: review code that creates temporary buffers with MAX_VERTICES, MAX_INDICES etc (waste of memory)
// TODO: use intrusive list for buffer (used slots, next vertex offset etc)
// TODO: refactor merge to allow arbitrary selection
// TODO: remove color hacks when lighting is implemented
// TODO: review make_cylinder and make_pyramid (there must be a cleaner way)

constexpr int MAX_VERTICES = 8000;
constexpr int MAX_INDICES = 24000;
constexpr int MAX_REFS = 2000;

typedef int Ref;

struct DrawCommand
{
    unsigned int indicesCount;
    unsigned int copies = 1;
    unsigned int indexOffset;
    int vertexOffset;
    unsigned int baseInstance = 0;
};

struct Buffer
{
    Vec4 vertices[MAX_VERTICES] = {};
    unsigned int indices[MAX_INDICES] = {};
    Color faceColors[MAX_INDICES / 3] = {};
    int faceOffsets[MAX_REFS] = {};
    Mat4 models[MAX_REFS] = {};
    DrawCommand drawCmds[MAX_REFS] = {};
    bool usedSlots[MAX_REFS] = {};

    int vtxCount = 0;
    int idxCount = 0;
    int slotCount = 1;

    bool meshDirty = false;
    bool modelsDirty = false;
};

namespace buffer
{

void reset(Buffer &buf);
bool can_add(const Buffer &buf, const Mesh &mesh);
Ref add(Buffer &buf, const Mesh &mesh);
void remove(Buffer &buf, Ref ref);
void move_to_end(Buffer &buf, Ref ref);
Ref merge(Buffer &buf, Ref dst, Ref src);
void recompose_model(Buffer &buf, Ref ref, const Mat4 &model);

} // namespace buffer

#endif // BUFFER_H
