#include "mesh_edit.h"

namespace mesh
{

static int get_touched_vertices(const Buffer &buf, const DrawCommand &cmd, int *faces, int faceCount, int *touched,
                                int offset)
{
    int touchedCount = 0;
    for (int f = 0; f < faceCount; f++)
    {
        int base = cmd.indexOffset + faces[f] * 3;
        for (int v = 0; v < 3; v++)
        {
            int vtx = offset + buf.indices[base + v];
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

void transform_faces(Buffer &buf, Ref ref, int *faces, int faceCount, Mat4 t)
{
    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = buf.drawCmds[ref];

    int touched[MAX_VERTICES];
    int touchedCount = get_touched_vertices(buf, cmd, faces, faceCount, touched, cmd.vertexOffset);

    Vec4 centroid = {};
    for (int i = 0; i < touchedCount; i++)
    {
        centroid += buf.vertices[touched[i]];
    }
    centroid *= (1.0f / touchedCount);

    for (int i = 0; i < touchedCount; i++)
    {
        Vec4 &vx = buf.vertices[touched[i]];
        Vec4 local = {vx.x - centroid.x, vx.y - centroid.y, vx.z - centroid.z, 1.0f};
        Vec4 result = t * local;
        vx.x = result.x + centroid.x;
        vx.y = result.y + centroid.y;
        vx.z = result.z + centroid.z;
    }

    buf.meshDirty = true;
}

void extrude_faces(Buffer &buf, Ref ref, int *faces, int faceCount)
{
    // extrude appends new vertices at vtxCount, which requires the ref to
    // be last in the vertex buffer
    buffer::move_to_end(buf, ref);

    if (faceCount == 0)
    {
        return;
    }

    DrawCommand &cmd = buf.drawCmds[ref];

    int touched[MAX_VERTICES];
    int touchedCount = get_touched_vertices(buf, cmd, faces, faceCount, touched, 0);

    // averaged face normal
    Vec4 normal = {};
    for (int f = 0; f < faceCount; f++)
    {
        int base = cmd.indexOffset + faces[f] * 3;
        Vec4 &a = buf.vertices[cmd.vertexOffset + buf.indices[base + 0]];
        Vec4 &b = buf.vertices[cmd.vertexOffset + buf.indices[base + 1]];
        Vec4 &c = buf.vertices[cmd.vertexOffset + buf.indices[base + 2]];
        normal += vec4::cross(b - a, c - a);
    }
    normal = vec4::normalize(normal);

    // directed boundary edges (appear in exactly one selected face)
    struct Edge
    {
        int a, b, face;
    };
    Edge dirEdges[MAX_INDICES];
    int edgeCount = 0;
    int edgeHits[MAX_INDICES] = {};
    for (int f = 0; f < faceCount; f++)
    {
        int base = cmd.indexOffset + faces[f] * 3;
        for (int e = 0; e < 3; e++)
        {
            int va = buf.indices[base + e];
            int vb = buf.indices[base + (e + 1) % 3];
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
                dirEdges[edgeCount] = {va, vb, f};
                edgeHits[edgeCount] = 1;
                edgeCount++;
            }
        }
    }

    // duplicate touched vertices (append to global array)
    int newLocalBase = buf.vtxCount - cmd.vertexOffset;
    for (int i = 0; i < touchedCount; i++)
    {
        buf.vertices[buf.vtxCount + i] = buf.vertices[cmd.vertexOffset + touched[i]];
    }

    // remap selected face indices to new vertices
    for (int f = 0; f < faceCount; f++)
    {
        int base = cmd.indexOffset + faces[f] * 3;
        for (int v = 0; v < 3; v++)
        {
            int old = buf.indices[base + v];
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == old)
                {
                    buf.indices[base + v] = newLocalBase + i;
                    break;
                }
            }
        }
    }

    // change extruded faces color
    Color currentColor = buf.faceColors[buf.faceOffsets[ref] + faces[0]];
    Color nextColor = color::next_color(currentColor);
    for (int f = 0; f < faceCount; f++)
    {
        buf.faceColors[buf.faceOffsets[ref] + faces[f]] = nextColor;
    }

    // append wall triangles with colors from palette
    int existingFaceCount = cmd.indicesCount / 3;
    int wallFaceIdx = 0;
    int wallColorIdx = 0;
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
        int wallBase = cmd.indexOffset + cmd.indicesCount + wallFaceIdx * 6;
        buf.indices[wallBase + 0] = va;
        buf.indices[wallBase + 1] = vb;
        buf.indices[wallBase + 2] = newB;
        buf.indices[wallBase + 3] = va;
        buf.indices[wallBase + 4] = newB;
        buf.indices[wallBase + 5] = newA;

        // cycle through palette skipping the extruded face color
        Color wallColor = color::palette[wallColorIdx % color::paletteSize];
        while (wallColor.r == nextColor.r && wallColor.g == nextColor.g && wallColor.b == nextColor.b)
        {
            wallColorIdx++;
            wallColor = color::palette[wallColorIdx % color::paletteSize];
        }

        buf.faceColors[buf.faceOffsets[ref] + existingFaceCount + wallFaceIdx * 2 + 0] = wallColor;
        buf.faceColors[buf.faceOffsets[ref] + existingFaceCount + wallFaceIdx * 2 + 1] = wallColor;
        wallColorIdx++;
        wallFaceIdx++;
    }

    // displace new vertices slightly along the face normal
    Vec4 offset = normal * 0.05f;
    for (int i = 0; i < touchedCount; i++)
    {
        buf.vertices[buf.vtxCount + i] += offset;
    }

    int added = wallFaceIdx * 6;
    buf.vtxCount += touchedCount;
    buf.idxCount += added;
    cmd.indicesCount += added;

    buf.meshDirty = true;
}
} // namespace mesh
