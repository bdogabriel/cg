#ifndef FACE_EDIT_H
#define FACE_EDIT_H

#include "mesh_batch.h"

namespace faceedit
{
void transform(MeshBatch &batch, Ref ref, int *faces, int faceCount, Mat4 t);
void extrude(MeshBatch &batch, Ref ref, int *faces, int faceCount);
} // namespace faceedit

#endif
