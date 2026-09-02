#include "editor.h"

namespace editor
{

void push_undo(Editor &e)
{
    int i = (e.undoHead + e.undoCount) % MAX_UNDO;
    e.undoBatches[i] = e.meshBatch;
    memcpy(e.undoTransforms[i], e.transforms, sizeof(e.transforms));
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
    e.meshBatch = e.undoBatches[i];
    memcpy(e.transforms, e.undoTransforms[i], sizeof(e.transforms));

    e.meshBatch.meshDirty = true;
    e.meshBatch.modelsDirty = true;
}

} // namespace editor
