#ifndef MESH_BATCH_H
#define MESH_BATCH_H

#include "face_batch.h"
#include "mesh.h"
#include "model_texture_cache.h"

// TODO: cycle faces sorted by distance to the camera (when camera is implemented)
// TODO: review code that creates temporary buffers with MAX_VERTICES, MAX_INDICES etc (waste of memory)
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
    Vec4 vertices[mesh::MAX_VERTICES] = {};
    Vec2 uvs[mesh::MAX_VERTICES] = {};
    FaceBatch faceBatch;

    // derived (rebuilt by triangulate)
    unsigned int indices[mesh::MAX_INDICES] = {};
    Color primColors[mesh::MAX_INDICES / 3] = {};
    int primMaterialIds[mesh::MAX_INDICES / 3] = {};
    int primOffsets[mesh::MAX_REFS] = {};
    bool texturedSlots[mesh::MAX_REFS] = {};

    Mat4 models[mesh::MAX_REFS] = {};
    DrawCommand drawCmds[mesh::MAX_REFS] = {};
    bool usedSlots[mesh::MAX_REFS] = {};

    // derived (rebuilt by build_commands)
    DrawCommand flatCmds[mesh::MAX_REFS] = {};
    DrawCommand texCmds[face::MAX_FACES] = {};
    int texMatStart[modeltexture::MAX_MODEL_TEXTURES] = {};
    int texMatCount[modeltexture::MAX_MODEL_TEXTURES] = {};
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

} // namespace mesh

#endif
