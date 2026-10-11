#include "mesh_batch.h"
#include <stdio.h>

namespace mesh
{

void reset(MeshBatch &batch)
{
    batch.vtxCount = 0;
    batch.faceBatch.faceCount = 0;
    batch.faceBatch.cornerCount = 0;
    batch.idxCount = 0;
    batch.primCount = 0;
    batch.slotCount = 1;
    memset(batch.usedSlots, 0, sizeof(batch.usedSlots));
    memset(batch.texturedSlots, 0, sizeof(batch.texturedSlots));
    batch.meshDirty = true;
    batch.modelsDirty = true;
}

bool can_add(const MeshBatch &batch, const Mesh &mesh)
{
    if (batch.slotCount >= mesh::MAX_REFS)
    {
        printf("E: cannot add ref, mesh::MAX_REFS limit reached (%d)\n", mesh::MAX_REFS);
        return false;
    }
    if (batch.vtxCount + mesh.vtxCount > mesh::MAX_VERTICES)
    {
        printf("E: cannot add %s, would exceed MAX_VERTICES (%d + %d > %d)\n", mesh.name, batch.vtxCount, mesh.vtxCount,
               mesh::MAX_VERTICES);
        return false;
    }
    if (batch.faceBatch.faceCount + mesh.faceCount > face::MAX_FACES)
    {
        printf("E: cannot add %s, would exceed MAX_FACES (%d + %d > %d)\n", mesh.name, batch.faceBatch.faceCount,
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
    if (batch.faceBatch.cornerCount + meshCorners > face::MAX_CORNERS)
    {
        printf("E: cannot add %s, would exceed MAX_CORNERS (%d + %d > %d)\n", mesh.name, batch.faceBatch.cornerCount,
               meshCorners, face::MAX_CORNERS);
        return false;
    }
    if (batch.idxCount + meshTris * 3 > mesh::MAX_INDICES)
    {
        printf("E: cannot add %s, would exceed MAX_INDICES (%d + %d > %d)\n", mesh.name, batch.idxCount, meshTris * 3,
               mesh::MAX_INDICES);
        return false;
    }
    return true;
}

Ref add(MeshBatch &batch, const Mesh &mesh)
{
    int vtxOffset = batch.vtxCount;
    int faceOffset = batch.faceBatch.faceCount;
    int cornerOffset = batch.faceBatch.cornerCount;

    memcpy(batch.vertices + batch.vtxCount, mesh.vertices, mesh.vtxCount * sizeof(Vec4));
    if (mesh.uvs)
    {
        memcpy(batch.uvs + batch.vtxCount, mesh.uvs, mesh.vtxCount * sizeof(Vec2));
    }

    int totalCorners = 0;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        totalCorners += mesh.faceCornerCounts[i];
    }
    if (mesh.faceCorners && totalCorners > 0)
    {
        memcpy(batch.faceBatch.faceCorners + batch.faceBatch.cornerCount, mesh.faceCorners,
               totalCorners * sizeof(unsigned int));
    }

    memcpy(batch.faceBatch.faceCornerCounts + batch.faceBatch.faceCount, mesh.faceCornerCounts,
           mesh.faceCount * sizeof(int));
    memcpy(batch.faceBatch.faceColors + batch.faceBatch.faceCount, mesh.faceColors, mesh.faceCount * sizeof(Color));
    if (mesh.faceMaterialIds)
    {
        memcpy(batch.faceBatch.faceMaterialIds + batch.faceBatch.faceCount, mesh.faceMaterialIds,
               mesh.faceCount * sizeof(int));
    }
    else
    {
        memset(batch.faceBatch.faceMaterialIds + batch.faceBatch.faceCount, 0xFF, mesh.faceCount * sizeof(int));
    }

    int runningCorner = cornerOffset;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        batch.faceBatch.faceCornerStarts[batch.faceBatch.faceCount + i] = runningCorner;
        runningCorner += mesh.faceCornerCounts[i];
    }

    batch.vtxCount += mesh.vtxCount;
    batch.faceBatch.faceCount += mesh.faceCount;
    batch.faceBatch.cornerCount += totalCorners;

    int slot = 1;
    while (slot < mesh::MAX_REFS && batch.usedSlots[slot])
    {
        slot++;
    }
    batch.usedSlots[slot] = true;
    if (slot >= batch.slotCount)
    {
        batch.slotCount = slot + 1;
    }

    batch.models[slot] = mesh.model;
    batch.faceBatch.faceOffsets[slot] = faceOffset;
    batch.faceBatch.faceCounts[slot] = mesh.faceCount;
    batch.texturedSlots[slot] = (mesh.uvs != nullptr);

    batch.drawCmds[slot] = {.indicesCount = 0,
                            .copies = 1,
                            .indexOffset = 0,
                            .vertexOffset = vtxOffset,
                            .baseInstance = (unsigned int)slot};

    batch.meshDirty = true;
    batch.modelsDirty = true;

    return slot;
}

void remove(MeshBatch &batch, Ref ref)
{
    batch.usedSlots[ref] = false;
    batch.drawCmds[ref].indicesCount = 0;
    batch.drawCmds[ref].indexOffset = 0;
    while (batch.slotCount > 1 && !batch.usedSlots[batch.slotCount - 1])
    {
        batch.slotCount--;
    }
}

void compose_model(MeshBatch &batch, Ref ref, const Mat4 &model)
{
    batch.models[ref] = model;
    batch.modelsDirty = true;
}

void triangulate(MeshBatch &batch)
{
    batch.idxCount = 0;
    batch.primCount = 0;

    for (int slot = 1; slot < batch.slotCount; slot++)
    {
        if (!batch.usedSlots[slot])
        {
            continue;
        }

        DrawCommand &cmd = batch.drawCmds[slot];
        cmd.indexOffset = batch.idxCount;
        batch.primOffsets[slot] = batch.primCount;

        int faceStart = batch.faceBatch.faceOffsets[slot];
        int faceEnd = faceStart + batch.faceBatch.faceCounts[slot];
        int vtxOffset = cmd.vertexOffset;

        for (int f = faceStart; f < faceEnd; f++)
        {
            int n = batch.faceBatch.faceCornerCounts[f];
            if (n < 3)
            {
                continue;
            }

            int cornerBase = batch.faceBatch.faceCornerStarts[f];

            if (n == 3)
            {
                batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase];
                batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase + 1];
                batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase + 2];
                batch.primColors[batch.primCount] = batch.faceBatch.faceColors[f];
                batch.primMaterialIds[batch.primCount] = batch.faceBatch.faceMaterialIds[f];
                batch.primCount++;
            }
            else
            {
                Vec4 loop[64];
                for (int i = 0; i < n; i++)
                {
                    loop[i] = batch.vertices[vtxOffset + batch.faceBatch.faceCorners[cornerBase + i]];
                }

                int outTris[3 * 64];
                int triCount = face::triangulate(loop, n, outTris);

                for (int t = 0; t < triCount; t++)
                {
                    batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 0]];
                    batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 1]];
                    batch.indices[batch.idxCount++] = batch.faceBatch.faceCorners[cornerBase + outTris[t * 3 + 2]];
                    batch.primColors[batch.primCount] = batch.faceBatch.faceColors[f];
                    batch.primMaterialIds[batch.primCount] = batch.faceBatch.faceMaterialIds[f];
                    batch.primCount++;
                }
            }
        }

        cmd.indicesCount = batch.idxCount - cmd.indexOffset;
    }
}

void build_commands(MeshBatch &batch, int textureCount)
{
    int flat = 0;
    int tex = 0;

    // flat: one command each.
    for (int slot = 1; slot < batch.slotCount; slot++)
    {
        if (!batch.usedSlots[slot])
        {
            continue;
        }
        if (!batch.texturedSlots[slot])
        {
            batch.flatCmds[flat++] = batch.drawCmds[slot];
        }
    }

    // textured: bucketed by material
    for (int m = 0; m < textureCount; m++)
    {
        batch.texMatStart[m] = tex;
        for (int slot = 1; slot < batch.slotCount; slot++)
        {
            if (!batch.usedSlots[slot] || !batch.texturedSlots[slot])
            {
                continue;
            }
            int triStart = batch.primOffsets[slot];
            int triEnd = triStart + (int)batch.drawCmds[slot].indicesCount / 3;
            for (int i = triStart; i < triEnd;)
            {
                int mm = batch.primMaterialIds[i];
                int j = i;
                while (j < triEnd && batch.primMaterialIds[j] == mm)
                {
                    j++;
                }
                if (mm == m)
                {
                    DrawCommand cmd = batch.drawCmds[slot];
                    cmd.indexOffset = (unsigned int)(i * 3);
                    cmd.indicesCount = (unsigned int)(j - i) * 3;
                    batch.texCmds[tex++] = cmd;
                }
                i = j;
            }
        }
        batch.texMatCount[m] = tex - batch.texMatStart[m];
    }

    batch.flatCount = flat;
    batch.texCount = tex;
}

} // namespace mesh
