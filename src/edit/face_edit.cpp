#include "mesh_edit.h"

#include "arena.h"

#include <stdio.h>

namespace faceedit
{

namespace
{

int get_touched_vertices(const MeshBatch &batch, Ref ref, int *faces, int faceCount, int *touched)
{
    int touchedCount = 0;
    int faceBase = BUF(int, batch.faceBatch.faceOffsets)[ref];
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[faceBase + faces[f]];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int vtx = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + v];
            bool found = false;
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == vtx)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                touched[touchedCount++] = vtx;
            }
        }
    }
    return touchedCount;
}

} // namespace

void transform(MeshBatch &batch, Ref ref, int *faces, int faceCount, Mat4 t)
{
    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = BUF(DrawCommand, batch.drawCmds)[ref];

    int *touched = (int *)arena::scratch_alloc(sizeof(int) * (size_t)batch.vtxCount);
    int touchedCount = get_touched_vertices(batch, ref, faces, faceCount, touched);

    Vec4 centroid = {};
    for (int i = 0; i < touchedCount; i++)
    {
        centroid += BUF(Vec4, batch.vertices)[cmd.vertexOffset + touched[i]];
    }
    centroid *= (1.0f / touchedCount);

    for (int i = 0; i < touchedCount; i++)
    {
        Vec4 &vx = BUF(Vec4, batch.vertices)[cmd.vertexOffset + touched[i]];
        Vec4 local = {vx.x - centroid.x, vx.y - centroid.y, vx.z - centroid.z, 1.0f};
        Vec4 result = t * local;
        vx.x = result.x + centroid.x;
        vx.y = result.y + centroid.y;
        vx.z = result.z + centroid.z;
    }

    batch.meshDirty = true;
}

void extrude(MeshBatch &batch, Ref ref, int *faces, int faceCount)
{
    meshedit::move_to_end(batch, ref);

    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = BUF(DrawCommand, batch.drawCmds)[ref];
    int faceBase = BUF(int, batch.faceBatch.faceOffsets)[ref];

    int *touched = (int *)arena::scratch_alloc(sizeof(int) * (size_t)batch.vtxCount);
    int touchedCount = get_touched_vertices(batch, ref, faces, faceCount, touched);

    Vec4 normal = {};
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[faceBase + faces[f]];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceBase + faces[f]];
        for (int i = 0; i < cornerCount; i++)
        {
            Vec4 &curr =
                BUF(Vec4, batch.vertices)[cmd.vertexOffset + BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + i]];
            Vec4 &next = BUF(Vec4, batch.vertices)[cmd.vertexOffset +
                                                     BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart +
                                                                                                     (i + 1) % cornerCount]];
            normal.x += (curr.y - next.y) * (curr.z + next.z);
            normal.y += (curr.z - next.z) * (curr.x + next.x);
            normal.z += (curr.x - next.x) * (curr.y + next.y);
        }
    }
    normal = vec4::normalize(normal);

    struct Edge
    {
        int a, b;
    };
    Edge *dirEdges = (Edge *)arena::scratch_alloc(sizeof(Edge) * (size_t)batch.faceBatch.cornerCount);
    int *edgeHits = (int *)arena::scratch_alloc(sizeof(int) * (size_t)batch.faceBatch.cornerCount);
    int edgeCount = 0;
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[faceBase + faces[f]];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceBase + faces[f]];
        for (int e = 0; e < cornerCount; e++)
        {
            int va = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + e];
            int vb = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + (e + 1) % cornerCount];
            int lo = va < vb ? va : vb;
            int hi = va < vb ? vb : va;
            bool found = false;
            for (int i = 0; i < edgeCount; i++)
            {
                int elo = dirEdges[i].a < dirEdges[i].b ? dirEdges[i].a : dirEdges[i].b;
                int ehi = dirEdges[i].a < dirEdges[i].b ? dirEdges[i].b : dirEdges[i].a;
                if (elo == lo && ehi == hi)
                {
                    edgeHits[i]++;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                dirEdges[edgeCount] = {va, vb};
                edgeHits[edgeCount] = 1;
                edgeCount++;
            }
        }
    }

    buffer::ensure(batch.vertices, sizeof(Vec4), batch.vtxCount + touchedCount);
    buffer::ensure(batch.faceBatch.faceCorners, sizeof(unsigned int), batch.faceBatch.cornerCount + edgeCount * 4);
    buffer::ensure(batch.faceBatch.faceCornerStarts, sizeof(int), batch.faceBatch.faceCount + edgeCount);
    buffer::ensure(batch.faceBatch.faceCornerCounts, sizeof(int), batch.faceBatch.faceCount + edgeCount);
    buffer::ensure(batch.faceBatch.faceColors, sizeof(Color), batch.faceBatch.faceCount + edgeCount);

    int newLocalBase = batch.vtxCount - cmd.vertexOffset;
    for (int i = 0; i < touchedCount; i++)
    {
        BUF(Vec4, batch.vertices)[batch.vtxCount + i] = BUF(Vec4, batch.vertices)[cmd.vertexOffset + touched[i]];
    }

    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[faceBase + faces[f]];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int old = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + v];
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == old)
                {
                    BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + v] = newLocalBase + i;
                    break;
                }
            }
        }
    }

    Color currentColor = BUF(Color, batch.faceBatch.faceColors)[faceBase + faces[0]];

    int wallFaceIdx = 0;
    for (int e = 0; e < edgeCount; e++)
    {
        if (edgeHits[e] != 1)
        {
            continue;
        }
        int va = dirEdges[e].a;
        int vb = dirEdges[e].b;
        int newA = newLocalBase, newB = newLocalBase;
        for (int i = 0; i < touchedCount; i++)
        {
            if (touched[i] == va)
            {
                newA = newLocalBase + i;
            }
            if (touched[i] == vb)
            {
                newB = newLocalBase + i;
            }
        }

        int cornerStart = batch.faceBatch.cornerCount;
        BUF(unsigned int, batch.faceBatch.faceCorners)[batch.faceBatch.cornerCount++] = va;
        BUF(unsigned int, batch.faceBatch.faceCorners)[batch.faceBatch.cornerCount++] = vb;
        BUF(unsigned int, batch.faceBatch.faceCorners)[batch.faceBatch.cornerCount++] = newB;
        BUF(unsigned int, batch.faceBatch.faceCorners)[batch.faceBatch.cornerCount++] = newA;
        BUF(int, batch.faceBatch.faceCornerStarts)[batch.faceBatch.faceCount] = cornerStart;
        BUF(int, batch.faceBatch.faceCornerCounts)[batch.faceBatch.faceCount] = 4;
        BUF(Color, batch.faceBatch.faceColors)[batch.faceBatch.faceCount] = currentColor;
        batch.faceBatch.faceCount++;

        wallFaceIdx++;
    }

    BUF(int, batch.faceBatch.faceCounts)[ref] += wallFaceIdx;

    Vec4 offset = normal * 0.05f;
    for (int i = 0; i < touchedCount; i++)
    {
        BUF(Vec4, batch.vertices)[batch.vtxCount + i] += offset;
    }

    batch.vtxCount += touchedCount;

    batch.meshDirty = true;
}

} // namespace faceedit
