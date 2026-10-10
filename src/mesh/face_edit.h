#ifndef FACE_EDIT_H
#define FACE_EDIT_H

#include "mesh_batch.h"

namespace face
{
void transform(MeshBatch &buf, Ref ref, int *faces, int faceCount, Mat4 t);
void extrude(MeshBatch &buf, Ref ref, int *faces, int faceCount);
int triangulate(const Vec4 *loop, int count, int *outTris);
} // namespace face

#endif // FACE_EDIT_H
