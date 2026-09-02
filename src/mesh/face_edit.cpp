#include "face_edit.h"
#include <cmath>
#include <stdio.h>

namespace face
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
    mesh::move_to_end(buf, ref);

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

static Vec4 compute_normal(const Vec4 *loop, int count)
{
    Vec4 normal = {0, 0, 0, 1};
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        normal.x += (loop[i].y - loop[j].y) * (loop[i].z + loop[j].z);
        normal.y += (loop[i].z - loop[j].z) * (loop[i].x + loop[j].x);
        normal.z += (loop[i].x - loop[j].x) * (loop[i].y + loop[j].y);
    }
    return normal;
}

static void drop_dominant_axis(const Vec4 *loop, int count, float *out2d, int &axis_x, int &axis_y)
{
    Vec4 normal = compute_normal(loop, count);
    float abs_x = std::abs(normal.x);
    float abs_y = std::abs(normal.y);
    float abs_z = std::abs(normal.z);

    if (abs_x >= abs_y && abs_x >= abs_z)
    {
        axis_x = 1;
        axis_y = 2;
    }
    else if (abs_y >= abs_x && abs_y >= abs_z)
    {
        axis_x = 0;
        axis_y = 2;
    }
    else
    {
        axis_x = 0;
        axis_y = 1;
    }

    for (int i = 0; i < count; i++)
    {
        out2d[i * 2] = (&loop[i].x)[axis_x];
        out2d[i * 2 + 1] = (&loop[i].x)[axis_y];
    }
}

static float compute_signed_area_2d(const float *poly2d, int count)
{
    float area = 0;
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        area += poly2d[i * 2] * poly2d[j * 2 + 1];
        area -= poly2d[j * 2] * poly2d[i * 2 + 1];
    }
    return area * 0.5f;
}

static bool is_point_in_triangle_2d(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
    float d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
    float d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

static bool is_ear(const float *poly2d, int count, const int *remaining, int i)
{
    int prev = (i + count - 1) % count;
    int next = (i + 1) % count;

    int a = remaining[prev];
    int b = remaining[i];
    int c = remaining[next];

    float ax = poly2d[a * 2], ay = poly2d[a * 2 + 1];
    float bx = poly2d[b * 2], by = poly2d[b * 2 + 1];
    float cx = poly2d[c * 2], cy = poly2d[c * 2 + 1];

    float cross = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    if (cross <= 0)
        return false;

    for (int j = 0; j < count; j++)
    {
        if (j == prev || j == i || j == next)
            continue;

        int v = remaining[j];
        float vx = poly2d[v * 2], vy = poly2d[v * 2 + 1];

        if (is_point_in_triangle_2d(vx, vy, ax, ay, bx, by, cx, cy))
            return false;
    }

    return true;
}

int triangulate(const Vec4 *loop, int count, int *outTris)
{
    if (count < 3)
        return 0;

    if (count == 3)
    {
        outTris[0] = 0;
        outTris[1] = 1;
        outTris[2] = 2;
        return 1;
    }

    float poly2d[128];
    int axis_x, axis_y;
    drop_dominant_axis(loop, count, poly2d, axis_x, axis_y);

    float area = compute_signed_area_2d(poly2d, count);

    int remaining[64];
    for (int i = 0; i < count; i++)
        remaining[i] = i;

    if (area < 0)
    {
        for (int i = 0; i < count / 2; i++)
        {
            int j = count - 1 - i;
            int tmp = remaining[i];
            remaining[i] = remaining[j];
            remaining[j] = tmp;
        }
    }

    int tri_count = 0;
    int remaining_count = count;

    while (remaining_count > 3)
    {
        bool found_ear = false;

        for (int i = 0; i < remaining_count; i++)
        {
            if (is_ear(poly2d, remaining_count, remaining, i))
            {
                int prev = (i + remaining_count - 1) % remaining_count;
                int next = (i + 1) % remaining_count;

                outTris[tri_count * 3 + 0] = remaining[prev];
                outTris[tri_count * 3 + 1] = remaining[i];
                outTris[tri_count * 3 + 2] = remaining[next];
                tri_count++;

                for (int j = i; j < remaining_count - 1; j++)
                    remaining[j] = remaining[j + 1];
                remaining_count--;

                found_ear = true;
                break;
            }
        }

        if (!found_ear)
        {
            for (int i = 1; i < remaining_count - 1; i++)
            {
                outTris[tri_count * 3 + 0] = remaining[0];
                outTris[tri_count * 3 + 1] = remaining[i];
                outTris[tri_count * 3 + 2] = remaining[i + 1];
                tri_count++;
            }
            break;
        }
    }

    if (remaining_count == 3)
    {
        outTris[tri_count * 3 + 0] = remaining[0];
        outTris[tri_count * 3 + 1] = remaining[1];
        outTris[tri_count * 3 + 2] = remaining[2];
        tri_count++;
    }

    return tri_count;
}

} // namespace face
