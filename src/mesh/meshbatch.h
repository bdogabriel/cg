#ifndef MESHBATCH_H
#define MESHBATCH_H

#include "face_batch.h"
#include "mesh.h"

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
    FaceBatch faceBatch;

    // derived (rebuilt by triangulate)
    unsigned int indices[mesh::MAX_INDICES] = {};
    Color primColors[mesh::MAX_INDICES / 3] = {};
    int primOffsets[mesh::MAX_REFS] = {};

    Mat4 models[mesh::MAX_REFS] = {};
    DrawCommand drawCmds[mesh::MAX_REFS] = {};
    bool usedSlots[mesh::MAX_REFS] = {};

    int vtxCount = 0;
    int idxCount = 0;
    int primCount = 0;
    int slotCount = 1;

    bool meshDirty = false;
    bool modelsDirty = false;
};

namespace mesh
{

void reset(MeshBatch &buf);
bool can_add(const MeshBatch &buf, const Mesh &mesh);
Ref add(MeshBatch &buf, const Mesh &mesh);
void remove(MeshBatch &buf, Ref ref);
void move_to_end(MeshBatch &buf, Ref ref);
Ref merge(MeshBatch &buf, Ref dst, Ref src);
void compose_model(MeshBatch &buf, Ref ref, const Mat4 &model);
void triangulate(MeshBatch &buf);
int merge_coplanar(MeshBatch &buf, Ref ref, float angleTolDeg,
                   const int *faces = nullptr, int selFaceCount = 0);

} // namespace mesh

#endif
