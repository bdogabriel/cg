#include "editor.h"
#include "trs.h"

#include <string.h>

namespace editor
{

void push_undo(Editor &e)
{
    int i = (e.undoHead + e.undoCount) % MAX_UNDO;
    mesh::clone(e.undoMeshBatches[i], e.meshBatch);

    buffer::ensure(e.transforms, sizeof(TRS), e.meshBatch.slotCount);
    buffer::ensure(e.undoTransforms[i], sizeof(TRS), e.meshBatch.slotCount);
    memcpy(BUF(TRS, e.undoTransforms[i]), BUF(TRS, e.transforms), e.meshBatch.slotCount * sizeof(TRS));

    if (e.undoCount == MAX_UNDO)
    {
        e.undoHead = (e.undoHead + 1) % MAX_UNDO;
    }
    else
    {
        ++e.undoCount;
    }
}

void pop_undo(Editor &e)
{
    if (e.undoCount == 0)
    {
        return;
    }
    --e.undoCount;
    int i = (e.undoHead + e.undoCount) % MAX_UNDO;

    mesh::swap(e.meshBatch, e.undoMeshBatches[i]);
    buffer::swap(e.transforms, e.undoTransforms[i]);

    e.meshBatch.meshDirty = true;
    e.meshBatch.modelsDirty = true;
}

} // namespace editor
