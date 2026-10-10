#include "mesh_edit.h"
#include <stdio.h>

namespace faceedit
{

static int get_touched_vertices(const MeshBatch &buf, Ref ref, int *faces, int faceCount, int *touched)
{
    int touchedCount = 0;
    int faceBase = buf.faceBatch.faceOffsets[ref];
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int vtx = buf.faceBatch.faceCorners[cornerStart + v];
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

void transform(MeshBatch &buf, Ref ref, int *faces, int faceCount, Mat4 t)
{
    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = buf.drawCmds[ref];

    int touched[mesh::MAX_VERTICES];
    int touchedCount = get_touched_vertices(buf, ref, faces, faceCount, touched);

    Vec4 centroid = {};
    for (int i = 0; i < touchedCount; i++)
    {
        centroid += buf.vertices[cmd.vertexOffset + touched[i]];
    }
    centroid *= (1.0f / touchedCount);

    for (int i = 0; i < touchedCount; i++)
    {
        Vec4 &vx = buf.vertices[cmd.vertexOffset + touched[i]];
        Vec4 local = {vx.x - centroid.x, vx.y - centroid.y, vx.z - centroid.z, 1.0f};
        Vec4 result = t * local;
        vx.x = result.x + centroid.x;
        vx.y = result.y + centroid.y;
        vx.z = result.z + centroid.z;
    }

    buf.meshDirty = true;
}

void extrude(MeshBatch &buf, Ref ref, int *faces, int faceCount)
{
    meshedit::move_to_end(buf, ref);

    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = buf.drawCmds[ref];
    int faceBase = buf.faceBatch.faceOffsets[ref];

    int touched[mesh::MAX_VERTICES];
    int touchedCount = get_touched_vertices(buf, ref, faces, faceCount, touched);

    Vec4 normal = {};
    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int i = 0; i < cornerCount; i++)
        {
            Vec4 &curr = buf.vertices[cmd.vertexOffset + buf.faceBatch.faceCorners[cornerStart + i]];
            Vec4 &next =
                buf.vertices[cmd.vertexOffset + buf.faceBatch.faceCorners[cornerStart + (i + 1) % cornerCount]];
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
        int cornerStart = buf.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int e = 0; e < cornerCount; e++)
        {
            int va = buf.faceBatch.faceCorners[cornerStart + e];
            int vb = buf.faceBatch.faceCorners[cornerStart + (e + 1) % cornerCount];
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

    if (buf.vtxCount + touchedCount > mesh::MAX_VERTICES)
    {
        printf("E: extrude would exceed MAX_VERTICES (%d + %d > %d)\n", buf.vtxCount, touchedCount, mesh::MAX_VERTICES);
        return;
    }
    if (buf.faceBatch.cornerCount + edgeCount * 4 > face::MAX_CORNERS)
    {
        printf("E: extrude would exceed MAX_CORNERS (%d + %d > %d)\n", buf.faceBatch.cornerCount, edgeCount * 4,
               face::MAX_CORNERS);
        return;
    }
    if (buf.faceBatch.faceCount + edgeCount > face::MAX_FACES)
    {
        printf("E: extrude would exceed MAX_FACES (%d + %d > %d)\n", buf.faceBatch.faceCount, edgeCount,
               face::MAX_FACES);
        return;
    }

    int newLocalBase = buf.vtxCount - cmd.vertexOffset;
    for (int i = 0; i < touchedCount; i++)
    {
        buf.vertices[buf.vtxCount + i] = buf.vertices[cmd.vertexOffset + touched[i]];
    }

    for (int f = 0; f < faceCount; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[faceBase + faces[f]];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceBase + faces[f]];
        for (int v = 0; v < cornerCount; v++)
        {
            int old = buf.faceBatch.faceCorners[cornerStart + v];
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == old)
                {
                    buf.faceBatch.faceCorners[cornerStart + v] = newLocalBase + i;
                    break;
                }
            }
        }
    }

    Color currentColor = buf.faceBatch.faceColors[faceBase + faces[0]];

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

        int cornerStart = buf.faceBatch.cornerCount;
        buf.faceBatch.faceCorners[buf.faceBatch.cornerCount++] = va;
        buf.faceBatch.faceCorners[buf.faceBatch.cornerCount++] = vb;
        buf.faceBatch.faceCorners[buf.faceBatch.cornerCount++] = newB;
        buf.faceBatch.faceCorners[buf.faceBatch.cornerCount++] = newA;
        buf.faceBatch.faceCornerStarts[buf.faceBatch.faceCount] = cornerStart;
        buf.faceBatch.faceCornerCounts[buf.faceBatch.faceCount] = 4;
        buf.faceBatch.faceColors[buf.faceBatch.faceCount] = currentColor;
        buf.faceBatch.faceCount++;

        wallFaceIdx++;
    }

    buf.faceBatch.faceCounts[ref] += wallFaceIdx;

    Vec4 offset = normal * 0.05f;
    for (int i = 0; i < touchedCount; i++)
    {
        buf.vertices[buf.vtxCount + i] += offset;
    }

    buf.vtxCount += touchedCount;

    buf.meshDirty = true;
}

} // namespace faceedit
