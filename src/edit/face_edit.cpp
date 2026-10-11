#include "mesh_edit.h"
#include <stdio.h>

namespace faceedit
{

static int get_touched_vertices(const MeshBatch &batch, Ref ref, int *faces, int faceCount, int *touched)
{
    int touchedCount = 0;
    int faceBase = batch.faceBatch.faceOffsets[ref];
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = batch.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int vtx = batch.faceBatch.faceCorners[cornerStart + v];
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

void transform(MeshBatch &batch, Ref ref, int *faces, int faceCount, Mat4 t)
{
    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = batch.drawCmds[ref];

    int touched[mesh::MAX_VERTICES];
    int touchedCount = get_touched_vertices(batch, ref, faces, faceCount, touched);

    Vec4 centroid = {};
    for (int i = 0; i < touchedCount; i++)
    {
        centroid += batch.vertices[cmd.vertexOffset + touched[i]];
    }
    centroid *= (1.0f / touchedCount);

    for (int i = 0; i < touchedCount; i++)
    {
        Vec4 &vx = batch.vertices[cmd.vertexOffset + touched[i]];
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

    DrawCommand &cmd = batch.drawCmds[ref];
    int faceBase = batch.faceBatch.faceOffsets[ref];

    int touched[mesh::MAX_VERTICES];
    int touchedCount = get_touched_vertices(batch, ref, faces, faceCount, touched);

    Vec4 normal = {};
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = batch.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int i = 0; i < cornerCount; i++)
        {
            Vec4 &curr = batch.vertices[cmd.vertexOffset + batch.faceBatch.faceCorners[cornerStart + i]];
            Vec4 &next =
                batch.vertices[cmd.vertexOffset + batch.faceBatch.faceCorners[cornerStart + (i + 1) % cornerCount]];
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
    Edge dirEdges[face::MAX_CORNERS];
    int edgeCount = 0;
    int edgeHits[face::MAX_CORNERS] = {};
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = batch.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int e = 0; e < cornerCount; e++)
        {
            int va = batch.faceBatch.faceCorners[cornerStart + e];
            int vb = batch.faceBatch.faceCorners[cornerStart + (e + 1) % cornerCount];
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

    if (batch.vtxCount + touchedCount > mesh::MAX_VERTICES)
    {
        printf("E: extrude would exceed MAX_VERTICES (%d + %d > %d)\n", batch.vtxCount, touchedCount,
               mesh::MAX_VERTICES);
        return;
    }
    if (batch.faceBatch.cornerCount + edgeCount * 4 > face::MAX_CORNERS)
    {
        printf("E: extrude would exceed MAX_CORNERS (%d + %d > %d)\n", batch.faceBatch.cornerCount, edgeCount * 4,
               face::MAX_CORNERS);
        return;
    }
    if (batch.faceBatch.faceCount + edgeCount > face::MAX_FACES)
    {
        printf("E: extrude would exceed MAX_FACES (%d + %d > %d)\n", batch.faceBatch.faceCount, edgeCount,
               face::MAX_FACES);
        return;
    }

    int newLocalBase = batch.vtxCount - cmd.vertexOffset;
    for (int i = 0; i < touchedCount; i++)
    {
        batch.vertices[batch.vtxCount + i] = batch.vertices[cmd.vertexOffset + touched[i]];
    }

    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = batch.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int old = batch.faceBatch.faceCorners[cornerStart + v];
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == old)
                {
                    batch.faceBatch.faceCorners[cornerStart + v] = newLocalBase + i;
                    break;
                }
            }
        }
    }

    Color currentColor = batch.faceBatch.faceColors[faceBase + faces[0]];

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
        batch.faceBatch.faceCorners[batch.faceBatch.cornerCount++] = va;
        batch.faceBatch.faceCorners[batch.faceBatch.cornerCount++] = vb;
        batch.faceBatch.faceCorners[batch.faceBatch.cornerCount++] = newB;
        batch.faceBatch.faceCorners[batch.faceBatch.cornerCount++] = newA;
        batch.faceBatch.faceCornerStarts[batch.faceBatch.faceCount] = cornerStart;
        batch.faceBatch.faceCornerCounts[batch.faceBatch.faceCount] = 4;
        batch.faceBatch.faceColors[batch.faceBatch.faceCount] = currentColor;
        batch.faceBatch.faceCount++;

        wallFaceIdx++;
    }

    batch.faceBatch.faceCounts[ref] += wallFaceIdx;

    Vec4 offset = normal * 0.05f;
    for (int i = 0; i < touchedCount; i++)
    {
        batch.vertices[batch.vtxCount + i] += offset;
    }

    batch.vtxCount += touchedCount;

    batch.meshDirty = true;
}

} // namespace faceedit
