#include "mesh_batch.h"

#include "arena.h"

#include <string.h>

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

    // slot-indexed arrays must always be valid for [0, slotCount), even when empty
    buffer::ensure(batch.usedSlots, sizeof(bool), batch.slotCount);
    buffer::ensure(batch.texturedSlots, sizeof(bool), batch.slotCount);
    buffer::ensure(batch.models, sizeof(Mat4), batch.slotCount);
    buffer::ensure(batch.drawCmds, sizeof(DrawCommand), batch.slotCount);
    buffer::ensure(batch.primOffsets, sizeof(int), batch.slotCount);
    buffer::ensure(batch.faceBatch.faceOffsets, sizeof(int), batch.slotCount);
    buffer::ensure(batch.faceBatch.faceCounts, sizeof(int), batch.slotCount);

    memset(BUF(bool, batch.usedSlots), 0, (size_t)batch.usedSlots.cap * sizeof(bool));
    memset(BUF(bool, batch.texturedSlots), 0, (size_t)batch.texturedSlots.cap * sizeof(bool));
    batch.meshDirty = true;
    batch.modelsDirty = true;
}

bool can_add(const MeshBatch &batch, const Mesh &mesh)
{
    (void)batch;
    (void)mesh;
    return true; // no fixed limits; buffers grow on demand in add()
}

Ref add(MeshBatch &batch, const Mesh &mesh)
{
    int vtxOffset = batch.vtxCount;
    int faceOffset = batch.faceBatch.faceCount;
    int cornerOffset = batch.faceBatch.cornerCount;

    int totalCorners = 0;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        totalCorners += mesh.faceCornerCounts[i];
    }

    // grow source arrays
    buffer::ensure(batch.vertices, sizeof(Vec4), batch.vtxCount + mesh.vtxCount);
    if (mesh.uvs)
    {
        buffer::ensure(batch.uvs, sizeof(Vec2), batch.vtxCount + mesh.vtxCount);
    }
    buffer::ensure(batch.faceBatch.faceCorners, sizeof(unsigned int), batch.faceBatch.cornerCount + totalCorners);
    buffer::ensure(batch.faceBatch.faceCornerStarts, sizeof(int), batch.faceBatch.faceCount + mesh.faceCount);
    buffer::ensure(batch.faceBatch.faceCornerCounts, sizeof(int), batch.faceBatch.faceCount + mesh.faceCount);
    buffer::ensure(batch.faceBatch.faceColors, sizeof(Color), batch.faceBatch.faceCount + mesh.faceCount);
    buffer::ensure(batch.faceBatch.faceMaterialIds, sizeof(int), batch.faceBatch.faceCount + mesh.faceCount);

    // grow slot arrays (slot scan below can touch up to slotCount, then extend by one)
    buffer::ensure(batch.usedSlots, sizeof(bool), batch.slotCount + 1);
    buffer::ensure(batch.texturedSlots, sizeof(bool), batch.slotCount + 1);
    buffer::ensure(batch.models, sizeof(Mat4), batch.slotCount + 1);
    buffer::ensure(batch.drawCmds, sizeof(DrawCommand), batch.slotCount + 1);
    buffer::ensure(batch.primOffsets, sizeof(int), batch.slotCount + 1);
    buffer::ensure(batch.faceBatch.faceOffsets, sizeof(int), batch.slotCount + 1);
    buffer::ensure(batch.faceBatch.faceCounts, sizeof(int), batch.slotCount + 1);

    memcpy(BUF(Vec4, batch.vertices) + batch.vtxCount, mesh.vertices, mesh.vtxCount * sizeof(Vec4));
    if (mesh.uvs)
    {
        memcpy(BUF(Vec2, batch.uvs) + batch.vtxCount, mesh.uvs, mesh.vtxCount * sizeof(Vec2));
    }

    if (mesh.faceCorners && totalCorners > 0)
    {
        memcpy(BUF(unsigned int, batch.faceBatch.faceCorners) + batch.faceBatch.cornerCount, mesh.faceCorners,
               totalCorners * sizeof(unsigned int));
    }

    memcpy(BUF(int, batch.faceBatch.faceCornerCounts) + batch.faceBatch.faceCount, mesh.faceCornerCounts,
           mesh.faceCount * sizeof(int));
    memcpy(BUF(Color, batch.faceBatch.faceColors) + batch.faceBatch.faceCount, mesh.faceColors,
           mesh.faceCount * sizeof(Color));
    if (mesh.faceMaterialIds)
    {
        memcpy(BUF(int, batch.faceBatch.faceMaterialIds) + batch.faceBatch.faceCount, mesh.faceMaterialIds,
               mesh.faceCount * sizeof(int));
    }
    else
    {
        memset(BUF(int, batch.faceBatch.faceMaterialIds) + batch.faceBatch.faceCount, 0xFF,
               mesh.faceCount * sizeof(int));
    }

    int runningCorner = cornerOffset;
    for (int i = 0; i < mesh.faceCount; i++)
    {
        BUF(int, batch.faceBatch.faceCornerStarts)[batch.faceBatch.faceCount + i] = runningCorner;
        runningCorner += mesh.faceCornerCounts[i];
    }

    batch.vtxCount += mesh.vtxCount;
    batch.faceBatch.faceCount += mesh.faceCount;
    batch.faceBatch.cornerCount += totalCorners;

    int slot = 1;
    while (BUF(bool, batch.usedSlots)[slot])
    {
        slot++;
    }
    BUF(bool, batch.usedSlots)[slot] = true;
    if (slot >= batch.slotCount)
    {
        batch.slotCount = slot + 1;
    }

    BUF(Mat4, batch.models)[slot] = mesh.model;
    BUF(int, batch.faceBatch.faceOffsets)[slot] = faceOffset;
    BUF(int, batch.faceBatch.faceCounts)[slot] = mesh.faceCount;
    BUF(bool, batch.texturedSlots)[slot] = (mesh.uvs != nullptr);

    BUF(DrawCommand, batch.drawCmds)
    [slot] = {.indicesCount = 0,
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
    BUF(bool, batch.usedSlots)[ref] = false;
    BUF(DrawCommand, batch.drawCmds)[ref].indicesCount = 0;
    BUF(DrawCommand, batch.drawCmds)[ref].indexOffset = 0;
    while (batch.slotCount > 1 && !BUF(bool, batch.usedSlots)[batch.slotCount - 1])
    {
        batch.slotCount--;
    }
}

void compose_model(MeshBatch &batch, Ref ref, const Mat4 &model)
{
    BUF(Mat4, batch.models)[ref] = model;
    batch.modelsDirty = true;
}

void triangulate(MeshBatch &batch)
{
    batch.idxCount = 0;
    batch.primCount = 0;

    // compute worst-case triangle count so the derived arrays can be grown in one shot
    int totalTris = 0;
    for (int slot = 1; slot < batch.slotCount; slot++)
    {
        if (!BUF(bool, batch.usedSlots)[slot])
        {
            continue;
        }
        int faceStart = BUF(int, batch.faceBatch.faceOffsets)[slot];
        int faceEnd = faceStart + BUF(int, batch.faceBatch.faceCounts)[slot];
        for (int f = faceStart; f < faceEnd; f++)
        {
            int n = BUF(int, batch.faceBatch.faceCornerCounts)[f];
            if (n >= 3)
            {
                totalTris += n - 2;
            }
        }
    }

    buffer::ensure(batch.indices, sizeof(unsigned int), totalTris * 3);
    buffer::ensure(batch.primColors, sizeof(Color), totalTris);
    buffer::ensure(batch.primMaterialIds, sizeof(int), totalTris);
    buffer::ensure(batch.primOffsets, sizeof(int), batch.slotCount);

    for (int slot = 1; slot < batch.slotCount; slot++)
    {
        if (!BUF(bool, batch.usedSlots)[slot])
        {
            continue;
        }

        DrawCommand &cmd = BUF(DrawCommand, batch.drawCmds)[slot];
        cmd.indexOffset = batch.idxCount;
        BUF(int, batch.primOffsets)[slot] = batch.primCount;

        int faceStart = BUF(int, batch.faceBatch.faceOffsets)[slot];
        int faceEnd = faceStart + BUF(int, batch.faceBatch.faceCounts)[slot];
        int vtxOffset = cmd.vertexOffset;

        for (int f = faceStart; f < faceEnd; f++)
        {
            int n = BUF(int, batch.faceBatch.faceCornerCounts)[f];
            if (n < 3)
            {
                continue;
            }

            int cornerBase = BUF(int, batch.faceBatch.faceCornerStarts)[f];
            unsigned int *corners = BUF(unsigned int, batch.faceBatch.faceCorners);

            if (n == 3)
            {
                BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase];
                BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase + 1];
                BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase + 2];
                BUF(Color, batch.primColors)[batch.primCount] = BUF(Color, batch.faceBatch.faceColors)[f];
                BUF(int, batch.primMaterialIds)[batch.primCount] = BUF(int, batch.faceBatch.faceMaterialIds)[f];
                batch.primCount++;
            }
            else
            {
                Vec4 *loop = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * n);
                int *outTris = (int *)arena::scratch_alloc(sizeof(int) * 3 * (n - 2));
                for (int i = 0; i < n; i++)
                {
                    loop[i] = BUF(Vec4, batch.vertices)[vtxOffset + corners[cornerBase + i]];
                }

                int triCount = face::triangulate(loop, n, outTris);

                for (int t = 0; t < triCount; t++)
                {
                    BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase + outTris[t * 3 + 0]];
                    BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase + outTris[t * 3 + 1]];
                    BUF(unsigned int, batch.indices)[batch.idxCount++] = corners[cornerBase + outTris[t * 3 + 2]];
                    BUF(Color, batch.primColors)[batch.primCount] = BUF(Color, batch.faceBatch.faceColors)[f];
                    BUF(int, batch.primMaterialIds)[batch.primCount] = BUF(int, batch.faceBatch.faceMaterialIds)[f];
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

    buffer::ensure(batch.flatCmds, sizeof(DrawCommand), batch.slotCount);
    buffer::ensure(batch.texCmds, sizeof(DrawCommand), batch.primCount);
    buffer::ensure(batch.texMatStart, sizeof(int), textureCount);
    buffer::ensure(batch.texMatCount, sizeof(int), textureCount);

    // flat: one command each
    for (int slot = 1; slot < batch.slotCount; slot++)
    {
        if (!BUF(bool, batch.usedSlots)[slot])
        {
            continue;
        }
        if (!BUF(bool, batch.texturedSlots)[slot])
        {
            BUF(DrawCommand, batch.flatCmds)[flat++] = BUF(DrawCommand, batch.drawCmds)[slot];
        }
    }

    // textured: bucketed by material
    for (int m = 0; m < textureCount; m++)
    {
        BUF(int, batch.texMatStart)[m] = tex;
        for (int slot = 1; slot < batch.slotCount; slot++)
        {
            if (!BUF(bool, batch.usedSlots)[slot] || !BUF(bool, batch.texturedSlots)[slot])
            {
                continue;
            }
            int triStart = BUF(int, batch.primOffsets)[slot];
            int triEnd = triStart + (int)BUF(DrawCommand, batch.drawCmds)[slot].indicesCount / 3;
            for (int i = triStart; i < triEnd;)
            {
                int mm = BUF(int, batch.primMaterialIds)[i];
                int j = i;
                while (j < triEnd && BUF(int, batch.primMaterialIds)[j] == mm)
                {
                    j++;
                }
                if (mm == m)
                {
                    DrawCommand cmd = BUF(DrawCommand, batch.drawCmds)[slot];
                    cmd.indexOffset = (unsigned int)(i * 3);
                    cmd.indicesCount = (unsigned int)(j - i) * 3;
                    BUF(DrawCommand, batch.texCmds)[tex++] = cmd;
                }
                i = j;
            }
        }
        BUF(int, batch.texMatCount)[m] = tex - BUF(int, batch.texMatStart)[m];
    }

    batch.flatCount = flat;
    batch.texCount = tex;
}

void clone(MeshBatch &dst, const MeshBatch &src)
{
    buffer::ensure(dst.vertices, sizeof(Vec4), src.vtxCount);
    if (src.vtxCount)
    {
        memcpy(BUF(Vec4, dst.vertices), BUF(Vec4, src.vertices), src.vtxCount * sizeof(Vec4));
    }
    buffer::ensure(dst.uvs, sizeof(Vec2), src.vtxCount);
    if (src.uvs.data && src.vtxCount)
    {
        memcpy(BUF(Vec2, dst.uvs), BUF(Vec2, src.uvs), src.vtxCount * sizeof(Vec2));
    }

    FaceBatch &df = dst.faceBatch;
    const FaceBatch &sf = src.faceBatch;

    buffer::ensure(df.faceCorners, sizeof(unsigned int), sf.cornerCount);
    if (sf.cornerCount)
    {
        memcpy(BUF(unsigned int, df.faceCorners), BUF(unsigned int, sf.faceCorners),
               sf.cornerCount * sizeof(unsigned int));
    }
    buffer::ensure(df.faceCornerStarts, sizeof(int), sf.faceCount);
    if (sf.faceCount)
    {
        memcpy(BUF(int, df.faceCornerStarts), BUF(int, sf.faceCornerStarts), sf.faceCount * sizeof(int));
    }
    buffer::ensure(df.faceCornerCounts, sizeof(int), sf.faceCount);
    if (sf.faceCount)
    {
        memcpy(BUF(int, df.faceCornerCounts), BUF(int, sf.faceCornerCounts), sf.faceCount * sizeof(int));
    }
    buffer::ensure(df.faceColors, sizeof(Color), sf.faceCount);
    if (sf.faceCount)
    {
        memcpy(BUF(Color, df.faceColors), BUF(Color, sf.faceColors), sf.faceCount * sizeof(Color));
    }
    buffer::ensure(df.faceMaterialIds, sizeof(int), sf.faceCount);
    if (sf.faceCount)
    {
        memcpy(BUF(int, df.faceMaterialIds), BUF(int, sf.faceMaterialIds), sf.faceCount * sizeof(int));
    }

    buffer::ensure(df.faceOffsets, sizeof(int), src.slotCount);
    buffer::ensure(df.faceCounts, sizeof(int), src.slotCount);
    buffer::ensure(dst.models, sizeof(Mat4), src.slotCount);
    buffer::ensure(dst.drawCmds, sizeof(DrawCommand), src.slotCount);
    buffer::ensure(dst.usedSlots, sizeof(bool), src.slotCount);
    buffer::ensure(dst.texturedSlots, sizeof(bool), src.slotCount);

    if (src.slotCount)
    {
        memcpy(BUF(int, df.faceOffsets), BUF(int, sf.faceOffsets), src.slotCount * sizeof(int));
        memcpy(BUF(int, df.faceCounts), BUF(int, sf.faceCounts), src.slotCount * sizeof(int));
        memcpy(BUF(Mat4, dst.models), BUF(Mat4, src.models), src.slotCount * sizeof(Mat4));
        memcpy(BUF(DrawCommand, dst.drawCmds), BUF(DrawCommand, src.drawCmds), src.slotCount * sizeof(DrawCommand));
        memcpy(BUF(bool, dst.usedSlots), BUF(bool, src.usedSlots), src.slotCount * sizeof(bool));
        memcpy(BUF(bool, dst.texturedSlots), BUF(bool, src.texturedSlots), src.slotCount * sizeof(bool));
    }

    // source counts (derived counts are rebuilt by triangulate/build_commands after restore)
    dst.vtxCount = src.vtxCount;
    df.faceCount = sf.faceCount;
    df.cornerCount = sf.cornerCount;
    dst.slotCount = src.slotCount;
    dst.idxCount = 0;
    dst.primCount = 0;
    dst.flatCount = 0;
    dst.texCount = 0;
    dst.meshDirty = src.meshDirty;
    dst.modelsDirty = src.modelsDirty;
}

void swap(MeshBatch &a, MeshBatch &b)
{
    MeshBatch tmp = a;
    a = b;
    b = tmp;
}

void free_buffers(MeshBatch &batch)
{
    buffer::free(batch.vertices);
    buffer::free(batch.uvs);
    buffer::free(batch.faceBatch.faceCorners);
    buffer::free(batch.faceBatch.faceCornerStarts);
    buffer::free(batch.faceBatch.faceCornerCounts);
    buffer::free(batch.faceBatch.faceColors);
    buffer::free(batch.faceBatch.faceMaterialIds);
    buffer::free(batch.faceBatch.faceOffsets);
    buffer::free(batch.faceBatch.faceCounts);
    buffer::free(batch.indices);
    buffer::free(batch.primColors);
    buffer::free(batch.primMaterialIds);
    buffer::free(batch.primOffsets);
    buffer::free(batch.texturedSlots);
    buffer::free(batch.models);
    buffer::free(batch.drawCmds);
    buffer::free(batch.usedSlots);
    buffer::free(batch.flatCmds);
    buffer::free(batch.texCmds);
    buffer::free(batch.texMatStart);
    buffer::free(batch.texMatCount);
}

} // namespace mesh
