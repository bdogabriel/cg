#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include "editor.h"

namespace wfront
{
bool load(MeshBatch &batch, Editor &e, const char *path, bool clearBuffer);
void save(const MeshBatch &batch, const ModelTextureCache &modelTexCache, Ref ref, const char *path);
} // namespace wfront

#endif
