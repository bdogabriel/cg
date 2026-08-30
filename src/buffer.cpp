#include "buffer.h"
#include <stdio.h>
#include <string.h>

namespace buffer
{

void reset(Buffer &buf)
{
    buf.vtxCount = 0;
    buf.idxCount = 0;
    buf.slotCount = 1;
    memset(buf.usedSlots, 0, sizeof(buf.usedSlots));
    buf.meshDirty = true;
    buf.modelsDirty = true;
}

bool can_add(const Buffer &buf, const Mesh &mesh)
{
    if (buf.slotCount >= MAX_REFS)
    {
        printf("E: cannot add ref, MAX_REFS limit reached (%d)\n", MAX_REFS);
        return false;
    }
    if (buf.vtxCount + mesh.vtxCount > MAX_VERTICES)
    {
        printf("E: cannot add %s, would exceed MAX_VERTICES (%d + %d > %d)\n", mesh.name, buf.vtxCount, mesh.vtxCount,
               MAX_VERTICES);
        return false;
    }
    if (buf.idxCount + mesh.idxCount > MAX_INDICES)
    {
        printf("E: cannot add %s, would exceed MAX_INDICES (%d + %d > %d)\n", mesh.name, buf.idxCount, mesh.idxCount,
               MAX_INDICES);
        return false;
    }
    return true;
}

Ref add(Buffer &buf, const Mesh &mesh)
{
    int vtxOffset = buf.vtxCount;
    int idxOffset = buf.idxCount;

    memcpy(buf.vertices + buf.vtxCount, mesh.vertices, mesh.vtxCount * sizeof(Vec4));
    memcpy(buf.indices + buf.idxCount, mesh.indices, mesh.idxCount * sizeof(unsigned int));

    buf.vtxCount += mesh.vtxCount;
    buf.idxCount += mesh.idxCount;

    // find first unused slot (slot 0 is reserved)
    int slot = 1;
    while (slot < MAX_REFS && buf.usedSlots[slot])
    {
        slot++;
    }
    buf.usedSlots[slot] = true;
    if (slot >= buf.slotCount)
    {
        buf.slotCount = slot + 1;
    }

    buf.models[slot] = mesh.model;

    buf.faceOffsets[slot] = idxOffset / mesh.faceVtxCount;
    memcpy(buf.faceColors + buf.faceOffsets[slot], mesh.faceColors, mesh.faceCount * sizeof(Color));

    buf.drawCmds[slot] = {.indicesCount = (unsigned int)mesh.idxCount,
                          .copies = 1,
                          .indexOffset = (unsigned int)idxOffset,
                          .vertexOffset = vtxOffset,
                          .baseInstance = 0};

    buf.meshDirty = true;
    buf.modelsDirty = true;

    return slot;
}

void remove(Buffer &buf, Ref ref)
{
    buf.usedSlots[ref] = false;
    buf.drawCmds[ref].indicesCount = 0;
    while (buf.slotCount > 1 && !buf.usedSlots[buf.slotCount - 1])
    {
        buf.slotCount--;
    }
}

void move_to_end(Buffer &buf, Ref ref)
{
    DrawCommand &cmd = buf.drawCmds[ref];
    if (cmd.indexOffset + cmd.indicesCount == (unsigned int)buf.idxCount)
    {
        return;
    }

    int origVtxOffset = cmd.vertexOffset;
    int origIdxOffset = cmd.indexOffset;
    int origFaceOffset = buf.faceOffsets[ref];

    int nextVtxOffset = buf.vtxCount;
    for (int j = 1; j < buf.slotCount; j++)
    {
        if (!buf.usedSlots[j] || j == ref)
        {
            continue;
        }
        if (buf.drawCmds[j].vertexOffset > origVtxOffset && buf.drawCmds[j].vertexOffset < nextVtxOffset)
        {
            nextVtxOffset = buf.drawCmds[j].vertexOffset;
        }
    }

    int movedVtxCount = nextVtxOffset - origVtxOffset;
    int movedIdxCount = (int)cmd.indicesCount;
    int movedFaceCount = movedIdxCount / 3;

    static Vec4 savedVerts[MAX_VERTICES];
    static unsigned int savedIndices[MAX_INDICES];
    static Color savedColors[MAX_INDICES / 3];

    memcpy(savedVerts, buf.vertices + origVtxOffset, movedVtxCount * sizeof(Vec4));
    memcpy(savedIndices, buf.indices + origIdxOffset, movedIdxCount * sizeof(unsigned int));
    memcpy(savedColors, buf.faceColors + origFaceOffset, movedFaceCount * sizeof(Color));

    // shift left
    memmove(buf.vertices + origVtxOffset, buf.vertices + origVtxOffset + movedVtxCount,
            (buf.vtxCount - origVtxOffset - movedVtxCount) * sizeof(Vec4));
    memmove(buf.indices + origIdxOffset, buf.indices + origIdxOffset + movedIdxCount,
            (buf.idxCount - origIdxOffset - movedIdxCount) * sizeof(unsigned int));
    memmove(buf.faceColors + origFaceOffset, buf.faceColors + origFaceOffset + movedFaceCount,
            (buf.idxCount / 3 - origFaceOffset - movedFaceCount) * sizeof(Color));

    // paste at tail
    int newVtxOffset = buf.vtxCount - movedVtxCount;
    int newIdxOffset = buf.idxCount - movedIdxCount;
    int newFaceOffset = buf.idxCount / 3 - movedFaceCount;

    memcpy(buf.vertices + newVtxOffset, savedVerts, movedVtxCount * sizeof(Vec4));
    memcpy(buf.indices + newIdxOffset, savedIndices, movedIdxCount * sizeof(unsigned int));
    memcpy(buf.faceColors + newFaceOffset, savedColors, movedFaceCount * sizeof(Color));

    // update offsets
    cmd.vertexOffset = newVtxOffset;
    cmd.indexOffset = newIdxOffset;
    buf.faceOffsets[ref] = newFaceOffset;

    for (int j = 1; j < buf.slotCount; j++)
    {
        if (!buf.usedSlots[j] || j == ref)
        {
            continue;
        }
        if (buf.drawCmds[j].vertexOffset > origVtxOffset)
        {
            buf.drawCmds[j].vertexOffset -= movedVtxCount;
        }
        if ((int)buf.drawCmds[j].indexOffset > origIdxOffset)
        {
            buf.drawCmds[j].indexOffset -= movedIdxCount;
        }
        if (buf.faceOffsets[j] > origFaceOffset)
        {
            buf.faceOffsets[j] -= movedFaceCount;
        }
    }
}

Ref merge(Buffer &buf, Ref dst, Ref src)
{
    move_to_end(buf, dst);
    move_to_end(buf, src);

    // transform src vertices into dst's local space
    Mat4 xform = mat4::inverse(buf.models[dst]) * buf.models[src];
    int srcVtxStart = buf.drawCmds[src].vertexOffset;
    int srcVtxCount = buf.vtxCount - srcVtxStart;
    for (int i = 0; i < srcVtxCount; i++)
    {
        buf.vertices[srcVtxStart + i] = xform * buf.vertices[srcVtxStart + i];
    }

    // move src indices
    int vtxRelOffset = buf.drawCmds[src].vertexOffset - buf.drawCmds[dst].vertexOffset;
    for (unsigned int i = 0; i < buf.drawCmds[src].indicesCount; i++)
    {
        buf.indices[buf.drawCmds[src].indexOffset + i] += (unsigned int)vtxRelOffset;
    }

    // expand dst's command
    buf.drawCmds[dst].indicesCount += buf.drawCmds[src].indicesCount;

    remove(buf, src);

    buf.meshDirty = true;
    buf.modelsDirty = true;

    return dst;
}

void recompose_model(Buffer &buf, Ref ref, const Mat4 &model)
{
    buf.models[ref] = model;
    buf.modelsDirty = true;
}

} // namespace buffer
