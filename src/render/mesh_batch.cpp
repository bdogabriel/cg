#include "mesh_batch.h"
#include <stdio.h>

namespace mesh
{

void reset(MeshBatch &buf)
{
    buf.vtxCount = 0;
    buf.faceBatch.faceCount = 0;
    buf.faceBatch.cornerCount = 0;
    buf.idxCount = 0;
    buf.primCount = 0;
    buf.slotCount = 1;
    memset(buf.usedSlots, 0, sizeof(buf.usedSlots));
    buf.meshDirty = true;
    buf.modelsDirty = true;
}

bool can_add(const MeshBatch &buf, const Mesh &mesh)
{
    if (buf.slotCount >= mesh::MAX_REFS)
    {
        printf("E: cannot add ref, mesh::MAX_REFS limit reached (%d)\n", mesh::MAX_REFS);
        return false;
    }
    if (buf.vtxCount + mesh.vtxCount > mesh::MAX_VERTICES)
    {
        printf("E: cannot add %s, would exceed MAX_VERTICES (%d + %d > %d)\n", mesh.name, buf.vtxCount, mesh.vtxCount,
               mesh::MAX_VERTICES);
        return false;
    }
    if (buf.faceBatch.faceCount + mesh.faceCount > face::MAX_FACES)
    {
        printf("E: cannot add %s, would exceed MAX_FACES (%d + %d > %d)\n", mesh.name, buf.faceBatch.faceCount,
               mesh.faceCount, face::MAX_FACES);
        return false;
    }
    int meshCorners = 0;
    int meshTris = 0;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        int n = mesh.faceCornerCounts[i];
        meshCorners += n;
        meshTris += (n >= 3) ? (n - 2) : 0;
    }
    if (buf.faceBatch.cornerCount + meshCorners > face::MAX_CORNERS)
    {
        printf("E: cannot add %s, would exceed MAX_CORNERS (%d + %d > %d)\n", mesh.name, buf.faceBatch.cornerCount,
               meshCorners, face::MAX_CORNERS);
        return false;
    }
    if (buf.idxCount + meshTris * 3 > mesh::MAX_INDICES)
    {
        printf("E: cannot add %s, would exceed MAX_INDICES (%d + %d > %d)\n", mesh.name, buf.idxCount, meshTris * 3,
               mesh::MAX_INDICES);
        return false;
    }
    return true;
}

Ref add(MeshBatch &buf, const Mesh &mesh)
{
    int vtxOffset = buf.vtxCount;
    int faceOffset = buf.faceBatch.faceCount;
    int cornerOffset = buf.faceBatch.cornerCount;

    memcpy(buf.vertices + buf.vtxCount, mesh.vertices, mesh.vtxCount * sizeof(Vec4));

    int totalCorners = 0;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        totalCorners += mesh.faceCornerCounts[i];
    }
    if (mesh.faceCorners && totalCorners > 0)
    {
        memcpy(buf.faceBatch.faceCorners + buf.faceBatch.cornerCount, mesh.faceCorners,
               totalCorners * sizeof(unsigned int));
    }

    memcpy(buf.faceBatch.faceCornerCounts + buf.faceBatch.faceCount, mesh.faceCornerCounts,
           mesh.faceCount * sizeof(int));
    memcpy(buf.faceBatch.faceColors + buf.faceBatch.faceCount, mesh.faceColors, mesh.faceCount * sizeof(Color));

    int runningCorner = cornerOffset;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        buf.faceBatch.faceCornerStarts[buf.faceBatch.faceCount + i] = runningCorner;
        runningCorner += mesh.faceCornerCounts[i];
    }

    buf.vtxCount += mesh.vtxCount;
    buf.faceBatch.faceCount += mesh.faceCount;
    buf.faceBatch.cornerCount += totalCorners;

    int slot = 1;
    while (slot < mesh::MAX_REFS && buf.usedSlots[slot])
    {
        slot++;
    }
    buf.usedSlots[slot] = true;
    if (slot >= buf.slotCount)
    {
        buf.slotCount = slot + 1;
    }

    buf.models[slot] = mesh.model;
    buf.faceBatch.faceOffsets[slot] = faceOffset;
    buf.faceBatch.faceCounts[slot] = mesh.faceCount;

    buf.drawCmds[slot] = {
        .indicesCount = 0, .copies = 1, .indexOffset = 0, .vertexOffset = vtxOffset, .baseInstance = 0};

    buf.meshDirty = true;
    buf.modelsDirty = true;

    return slot;
}

void remove(MeshBatch &buf, Ref ref)
{
    buf.usedSlots[ref] = false;
    buf.drawCmds[ref].indicesCount = 0;
    buf.drawCmds[ref].indexOffset = 0;
    while (buf.slotCount > 1 && !buf.usedSlots[buf.slotCount - 1])
    {
        buf.slotCount--;
    }
}

void compose_model(MeshBatch &buf, Ref ref, const Mat4 &model)
{
    buf.models[ref] = model;
    buf.modelsDirty = true;
}

void triangulate(MeshBatch &buf)
{
    buf.idxCount = 0;
    buf.primCount = 0;

    for (int slot = 1; slot < buf.slotCount; slot++)
    {
        if (!buf.usedSlots[slot])
        {
            continue;
        }

        DrawCommand &cmd = buf.drawCmds[slot];
        cmd.indexOffset = buf.idxCount;
        buf.primOffsets[slot] = buf.primCount;

        int faceStart = buf.faceBatch.faceOffsets[slot];
        int faceEnd = faceStart + buf.faceBatch.faceCounts[slot];
        int vtxOffset = cmd.vertexOffset;

        for (int f = faceStart; f < faceEnd; f++)
        {
            int n = buf.faceBatch.faceCornerCounts[f];
            if (n < 3)
            {
                continue;
            }

            int cornerBase = buf.faceBatch.faceCornerStarts[f];

            if (n == 3)
            {
                buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase];
                buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase + 1];
                buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase + 2];
                buf.primColors[buf.primCount++] = buf.faceBatch.faceColors[f];
            }
            else
            {
                Vec4 loop[64];
                for (int i = 0; i < n; i++)
                {
                    loop[i] = buf.vertices[vtxOffset + buf.faceBatch.faceCorners[cornerBase + i]];
                }

                int outTris[3 * 64];
                int triCount = face::triangulate(loop, n, outTris);

                for (int t = 0; t < triCount; t++)
                {
                    buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 0]];
                    buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 1]];
                    buf.indices[buf.idxCount++] = buf.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 2]];
                    buf.primColors[buf.primCount++] = buf.faceBatch.faceColors[f];
                }
            }
        }

        cmd.indicesCount = buf.idxCount - cmd.indexOffset;
    }
}

} // namespace mesh
