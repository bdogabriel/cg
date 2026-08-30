#ifndef MESH_EDIT_H
#define MESH_EDIT_H

#include "buffer.h"

namespace mesh
{
void transform_faces(Buffer &buf, Ref ref, int *faces, int faceCount, Mat4 t);
void extrude_faces(Buffer &buf, Ref ref, int *faces, int faceCount);
} // namespace mesh

#endif // MESH_EDIT_H
