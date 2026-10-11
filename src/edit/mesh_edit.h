#ifndef MESH_EDIT_H
#define MESH_EDIT_H

#include "mesh_batch.h"

namespace meshedit
{
void move_to_end(MeshBatch &batch, Ref ref);
Ref merge(MeshBatch &batch, Ref dst, Ref src);
int merge_coplanar(MeshBatch &batch, Ref ref, float angleTolDeg, const int *faces = nullptr, int selFaceCount = 0);
} // namespace meshedit

#endif
