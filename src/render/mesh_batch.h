#ifndef MESH_BATCH_H
#define MESH_BATCH_H

#include "buffer.h"
#include "face_batch.h"
#include "mesh.h"

// TODO: cycle faces sorted by distance to the camera (when camera is implemented)
// TODO: use intrusive list for buffer (used slots, next vertex offset etc)
// TODO: refactor merge to allow arbitrary selection
// TODO: review make_cylinder and make_pyramid (there must be a cleaner way)

typedef int Ref;

struct DrawCommand
{
    unsigned int indicesCount;
    unsigned int copies = 1;
    unsigned int indexOffset;
    int vertexOffset;
    unsigned int baseInstance = 0;
};

struct MeshBatch
{
    // source
    Buffer vertices; // Vec4
    Buffer uvs;      // Vec2
    FaceBatch faceBatch;

    // derived (rebuilt by triangulate)
    Buffer indices;         // unsigned int
    Buffer primColors;      // Color
    Buffer primMaterialIds; // int
    Buffer primOffsets;     // int; indexed by slot
    Buffer texturedSlots;   // bool; indexed by slot

    Buffer models;    // Mat4; indexed by slot
    Buffer drawCmds;  // DrawCommand, indexed by slot
    Buffer usedSlots; // bool; indexed by slot

    // derived (rebuilt by build_commands)
    Buffer flatCmds;    // DrawCommand
    Buffer texCmds;     // DrawCommand
    Buffer texMatStart; // int; indexed by texture
    Buffer texMatCount; // int; indexed by texture
    int flatCount = 0;
    int texCount = 0;

    int vtxCount = 0;
    int idxCount = 0;
    int primCount = 0;
    int slotCount = 1;

    bool meshDirty = false;
    bool modelsDirty = false;
};

namespace mesh
{

void reset(MeshBatch &batch);
bool can_add(const MeshBatch &batch, const Mesh &mesh);
Ref add(MeshBatch &batch, const Mesh &mesh);
void remove(MeshBatch &batch, Ref ref);
void compose_model(MeshBatch &batch, Ref ref, const Mat4 &model);
void triangulate(MeshBatch &batch);
void build_commands(MeshBatch &batch, int textureCount);

// deep-copy the used bytes of src into dst
// only source data is copied
// derived arrays are rebuilt by triangulate()/build_commands() after a restore
// used to snapshot undo
void clone(MeshBatch &dst, const MeshBatch &src);
// swap every buffer between two batches
// used to restore undo snapshots
void swap(MeshBatch &a, MeshBatch &b);
void free_buffers(MeshBatch &batch);

} // namespace mesh

#endif // MESH_BATCH_H
