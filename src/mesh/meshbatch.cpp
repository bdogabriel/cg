#include "meshbatch.h"
#include "face_edit.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void move_to_end(MeshBatch &buf, Ref ref)
{
    DrawCommand &cmd = buf.drawCmds[ref];

    int origVtxOffset = cmd.vertexOffset;
    int origFaceOffset = buf.faceBatch.faceOffsets[ref];
    int origFaceCount = buf.faceBatch.faceCounts[ref];

    int maxLocalVtx = 0;
    for (int f = origFaceOffset; f < origFaceOffset + origFaceCount; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[f];
        int cornerCount = buf.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cornerCount; c++)
        {
            int idx = buf.faceBatch.faceCorners[cornerStart + c];
            if (idx > maxLocalVtx)
            {
                maxLocalVtx = idx;
            }
        }
    }

    int nextVtxOffset = buf.vtxCount;
    for (int j = 1; j < buf.slotCount; j++)
    {
        if (!buf.usedSlots[j] || j == ref)
        {
            continue;
        }
        int jVtxOff = buf.drawCmds[j].vertexOffset;
        if (jVtxOff > origVtxOffset && jVtxOff < nextVtxOffset)
        {
            nextVtxOffset = jVtxOff;
        }
    }

    int movedVtxCount = nextVtxOffset - origVtxOffset;
    if (movedVtxCount == 0 && origFaceOffset + origFaceCount == buf.faceBatch.faceCount)
    {
        return;
    }

    int nextFaceOffset = buf.faceBatch.faceCount;
    for (int j = 1; j < buf.slotCount; j++)
    {
        if (!buf.usedSlots[j] || j == ref)
        {
            continue;
        }
        int jFaceOff = buf.faceBatch.faceOffsets[j];
        if (jFaceOff > origFaceOffset && jFaceOff < nextFaceOffset)
        {
            nextFaceOffset = jFaceOff;
        }
    }

    int refCornerStart = buf.faceBatch.faceCornerStarts[origFaceOffset];
    int nextCornerOffset = buf.faceBatch.cornerCount;
    if (origFaceOffset + origFaceCount < buf.faceBatch.faceCount)
    {
        nextCornerOffset = buf.faceBatch.faceCornerStarts[origFaceOffset + origFaceCount];
    }
    int movedCornerCount = nextCornerOffset - refCornerStart;

    static Vec4 savedVerts[mesh::MAX_VERTICES];
    static int savedCornerCounts[face::MAX_FACES];
    static Color savedColors[face::MAX_FACES];
    static unsigned int savedCorners[face::MAX_CORNERS];

    memcpy(savedVerts, buf.vertices + origVtxOffset, movedVtxCount * sizeof(Vec4));
    memcpy(savedCornerCounts, buf.faceBatch.faceCornerCounts + origFaceOffset, origFaceCount * sizeof(int));
    memcpy(savedColors, buf.faceBatch.faceColors + origFaceOffset, origFaceCount * sizeof(Color));
    memcpy(savedCorners, buf.faceBatch.faceCorners + refCornerStart, movedCornerCount * sizeof(unsigned int));

    memmove(buf.vertices + origVtxOffset, buf.vertices + origVtxOffset + movedVtxCount,
            (buf.vtxCount - origVtxOffset - movedVtxCount) * sizeof(Vec4));
    memmove(buf.faceBatch.faceCornerStarts + origFaceOffset,
            buf.faceBatch.faceCornerStarts + origFaceOffset + origFaceCount,
            (buf.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(buf.faceBatch.faceCornerCounts + origFaceOffset,
            buf.faceBatch.faceCornerCounts + origFaceOffset + origFaceCount,
            (buf.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(buf.faceBatch.faceColors + origFaceOffset, buf.faceBatch.faceColors + origFaceOffset + origFaceCount,
            (buf.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(Color));
    memmove(buf.faceBatch.faceCorners + refCornerStart, buf.faceBatch.faceCorners + refCornerStart + movedCornerCount,
            (buf.faceBatch.cornerCount - refCornerStart - movedCornerCount) * sizeof(unsigned int));

    int newVtxOffset = buf.vtxCount - movedVtxCount;
    int newFaceOffset = buf.faceBatch.faceCount - origFaceCount;
    int newCornerOffset = buf.faceBatch.cornerCount - movedCornerCount;

    memcpy(buf.vertices + newVtxOffset, savedVerts, movedVtxCount * sizeof(Vec4));
    memcpy(buf.faceBatch.faceCornerCounts + newFaceOffset, savedCornerCounts, origFaceCount * sizeof(int));
    memcpy(buf.faceBatch.faceColors + newFaceOffset, savedColors, origFaceCount * sizeof(Color));
    memcpy(buf.faceBatch.faceCorners + newCornerOffset, savedCorners, movedCornerCount * sizeof(unsigned int));

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
        if (buf.faceBatch.faceOffsets[j] > origFaceOffset)
        {
            buf.faceBatch.faceOffsets[j] -= origFaceCount;
        }
    }

    cmd.vertexOffset = newVtxOffset;
    buf.faceBatch.faceOffsets[ref] = newFaceOffset;

    face::update_corner_starts(buf.faceBatch);
}

Ref merge(MeshBatch &buf, Ref dst, Ref src)
{
    move_to_end(buf, dst);
    move_to_end(buf, src);

    DrawCommand &dstCmd = buf.drawCmds[dst];
    DrawCommand &srcCmd = buf.drawCmds[src];

    int srcVtxStart = srcCmd.vertexOffset;
    int srcVtxCount = buf.vtxCount - srcVtxStart;
    int dstVtxStart = dstCmd.vertexOffset;

    Mat4 xform = mat4::inverse(buf.models[dst]) * buf.models[src];
    for (int i = 0; i < srcVtxCount; i++)
    {
        buf.vertices[srcVtxStart + i] = xform * buf.vertices[srcVtxStart + i];
    }

    int vtxRelOffset = srcVtxStart - dstVtxStart;
    int srcFaceOffset = buf.faceBatch.faceOffsets[src];
    int srcFaceCount = buf.faceBatch.faceCounts[src];

    for (int f = srcFaceOffset; f < srcFaceOffset + srcFaceCount; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[f];
        int cornerCount = buf.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cornerCount; c++)
        {
            buf.faceBatch.faceCorners[cornerStart + c] += vtxRelOffset;
        }
    }

    buf.faceBatch.faceCounts[dst] += srcFaceCount;

    remove(buf, src);

    buf.meshDirty = true;
    buf.modelsDirty = true;

    return dst;
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

static Vec4 newell_normal(const Vec4 *verts, int count)
{
    Vec4 n = {0, 0, 0, 0};
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        const Vec4 &a = verts[i];
        const Vec4 &b = verts[j];
        n.x += (a.y - b.y) * (a.z + b.z);
        n.y += (a.z - b.z) * (a.x + b.x);
        n.z += (a.x - b.x) * (a.y + b.y);
    }
    return n;
}

struct HalfEdge
{
    unsigned int a, b;
    int face;
};

static HalfEdge halfEdges[face::MAX_CORNERS];

static int he_compare(const void *a, const void *b)
{
    const HalfEdge *ha = (const HalfEdge *)a;
    const HalfEdge *hb = (const HalfEdge *)b;
    unsigned int a_lo = ha->a < ha->b ? ha->a : ha->b;
    unsigned int a_hi = ha->a < ha->b ? ha->b : ha->a;
    unsigned int b_lo = hb->a < hb->b ? hb->a : hb->b;
    unsigned int b_hi = hb->a < hb->b ? hb->b : hb->a;
    if (a_lo != b_lo)
        return (a_lo < b_lo) ? -1 : 1;
    if (a_hi != b_hi)
        return (a_hi < b_hi) ? -1 : 1;
    return 0;
}

static int he_order_compare(const void *a, const void *b)
{
    int ia = *(const int *)a, ib = *(const int *)b;
    return he_compare(&halfEdges[ia], &halfEdges[ib]);
}

static bool he_same_canonical(const HalfEdge &a, const HalfEdge &b)
{
    unsigned int a_lo = a.a < a.b ? a.a : a.b;
    unsigned int a_hi = a.a < a.b ? a.b : a.a;
    unsigned int b_lo = b.a < b.b ? b.a : b.b;
    unsigned int b_hi = b.a < b.b ? b.b : b.a;
    return a_lo == b_lo && a_hi == b_hi;
}

int merge_coplanar(MeshBatch &buf, Ref ref, float angleTolDeg,
                   const int *faces, int selFaceCount)
{
    if (!buf.usedSlots[ref])
        return 0;

    int faceOffset = buf.faceBatch.faceOffsets[ref];
    int faceCount = buf.faceBatch.faceCounts[ref];
    if (faceCount <= 1)
        return faceCount;

    move_to_end(buf, ref);
    faceOffset = buf.faceBatch.faceOffsets[ref];
    faceCount = buf.faceBatch.faceCounts[ref];
    int vtxOffset = buf.drawCmds[ref].vertexOffset;

    static bool selected[face::MAX_FACES];
    memset(selected, 0, faceCount * sizeof(bool));
    if (faces == nullptr || selFaceCount <= 0)
    {
        for (int i = 0; i < faceCount; i++)
            selected[i] = true;
    }
    else
    {
        for (int i = 0; i < selFaceCount; i++)
        {
            int fi = faces[i];
            if (fi >= 0 && fi < faceCount)
                selected[fi] = true;
        }
    }

    float bboxMin[3] = {1e30f, 1e30f, 1e30f};
    float bboxMax[3] = {-1e30f, -1e30f, -1e30f};
    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = buf.faceBatch.faceCornerStarts[f];
        int cc = buf.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cc; c++)
        {
            const Vec4 &v = buf.vertices[vtxOffset + buf.faceBatch.faceCorners[cs + c]];
            for (int d = 0; d < 3; d++)
            {
                float val = (&v.x)[d];
                if (val < bboxMin[d])
                    bboxMin[d] = val;
                if (val > bboxMax[d])
                    bboxMax[d] = val;
            }
        }
    }
    float diag = sqrtf((bboxMax[0] - bboxMin[0]) * (bboxMax[0] - bboxMin[0]) +
                       (bboxMax[1] - bboxMin[1]) * (bboxMax[1] - bboxMin[1]) +
                       (bboxMax[2] - bboxMin[2]) * (bboxMax[2] - bboxMin[2]));
    float distEps = 1e-5f * (diag > 1.0f ? diag : 1.0f);
    float cosTol = cosf(angleTolDeg * PI / 180.0f);

    static Vec4 normals[face::MAX_FACES];
    static float offsets[face::MAX_FACES];
    static bool degenerate[face::MAX_FACES];

    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = buf.faceBatch.faceCornerStarts[f];
        int cc = buf.faceBatch.faceCornerCounts[f];

        Vec4 loop[64];
        for (int c = 0; c < cc; c++)
        {
            loop[c] = buf.vertices[vtxOffset + buf.faceBatch.faceCorners[cs + c]];
        }

        Vec4 n = newell_normal(loop, cc);
        float len = vec4::length(n);
        if (len < 1e-12f)
        {
            degenerate[i] = true;
            normals[i] = {0, 0, 0, 0};
            offsets[i] = 0;
        }
        else
        {
            degenerate[i] = false;
            normals[i] = {n.x / len, n.y / len, n.z / len, 0};
            offsets[i] = vec4::dot3(normals[i], loop[0]);
        }
    }

    int totalHE = 0;
    for (int i = 0; i < faceCount; i++)
    {
        totalHE += buf.faceBatch.faceCornerCounts[faceOffset + i];
    }

    int heIdx = 0;
    static int faceHEStart[face::MAX_FACES];
    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = buf.faceBatch.faceCornerStarts[f];
        int cc = buf.faceBatch.faceCornerCounts[f];
        faceHEStart[i] = heIdx;
        for (int c = 0; c < cc; c++)
        {
            unsigned int a = buf.faceBatch.faceCorners[cs + c];
            unsigned int b = buf.faceBatch.faceCorners[cs + (c + 1) % cc];
            halfEdges[heIdx++] = {a, b, i};
        }
    }

    static int order[face::MAX_CORNERS];
    for (int i = 0; i < totalHE; i++)
        order[i] = i;
    qsort(order, totalHE, sizeof(int), he_order_compare);

    static int reverse[face::MAX_CORNERS];
    for (int i = 0; i < totalHE; i++)
        reverse[i] = -1;

    int gi = 0;
    while (gi < totalHE)
    {
        int gj = gi + 1;
        while (gj < totalHE && he_same_canonical(halfEdges[order[gi]], halfEdges[order[gj]]))
            gj++;
        if (gj - gi == 2)
        {
            int h0 = order[gi], h1 = order[gi + 1];
            if (halfEdges[h0].a == halfEdges[h1].b && halfEdges[h0].b == halfEdges[h1].a)
            {
                reverse[h0] = h1;
                reverse[h1] = h0;
            }
        }
        gi = gj;
    }

    static int regionId[face::MAX_FACES];
    for (int i = 0; i < faceCount; i++)
        regionId[i] = -1;
    int regionCount = 0;

    static Vec4 seedNormals[face::MAX_FACES];
    static float seedOffsets[face::MAX_FACES];
    static int seedFace[face::MAX_FACES];

    static int queue[face::MAX_FACES];

    for (int start = 0; start < faceCount; start++)
    {
        if (!selected[start] || regionId[start] != -1 || degenerate[start])
            continue;

        int rid = regionCount++;
        regionId[start] = rid;
        seedNormals[rid] = normals[start];
        seedOffsets[rid] = offsets[start];
        seedFace[rid] = start;

        int qHead = 0, qTail = 0;
        queue[qTail++] = start;

        while (qHead < qTail)
        {
            int f = queue[qHead++];
            int heStart = faceHEStart[f];
            int heCount = buf.faceBatch.faceCornerCounts[faceOffset + f];

            for (int e = 0; e < heCount; e++)
            {
                int hi = heStart + e;
                int rev = reverse[hi];
                if (rev == -1)
                    continue;
                int partner = halfEdges[rev].face;
                if (partner == f || !selected[partner] || regionId[partner] != -1 || degenerate[partner])
                    continue;

                float dotN = vec4::dot3(seedNormals[rid], normals[partner]);
                if (dotN < cosTol)
                    continue;

                bool ok = true;
                int pcs = buf.faceBatch.faceCornerStarts[faceOffset + partner];
                int pcc = buf.faceBatch.faceCornerCounts[faceOffset + partner];
                for (int c = 0; c < pcc; c++)
                {
                    const Vec4 &v = buf.vertices[vtxOffset + buf.faceBatch.faceCorners[pcs + c]];
                    float dist = fabsf(vec4::dot3(seedNormals[rid], v) - seedOffsets[rid]);
                    if (dist > distEps)
                    {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;

                regionId[partner] = rid;
                queue[qTail++] = partner;
            }
        }
    }

    for (int i = 0; i < faceCount; i++)
    {
        if (selected[i] && degenerate[i] && regionId[i] == -1)
        {
            int rid = regionCount++;
            regionId[i] = rid;
            seedNormals[rid] = normals[i];
            seedOffsets[rid] = offsets[i];
            seedFace[rid] = i;
        }
    }

    static int heRegion[face::MAX_CORNERS];
    for (int i = 0; i < totalHE; i++)
    {
        heRegion[i] = regionId[halfEdges[i].face];
    }

    struct NewFace
    {
        unsigned int corners[64];
        int cornerCount;
        Color color;
    };
    static NewFace regionNewFaces[face::MAX_FACES];
    static bool regionEmitted[face::MAX_FACES];
    for (int rid = 0; rid < regionCount; rid++)
        regionEmitted[rid] = false;

    static int perimHE[face::MAX_CORNERS];
    static int startMap[mesh::MAX_VERTICES];
    static bool used[face::MAX_CORNERS];

    for (int rid = 0; rid < regionCount; rid++)
    {
        int perimCount = 0;
        for (int hi = 0; hi < totalHE; hi++)
        {
            if (heRegion[hi] != rid)
                continue;
            int rev = reverse[hi];
            if (rev == -1 || heRegion[rev] != rid)
            {
                perimHE[perimCount++] = hi;
            }
        }

        if (perimCount == 0)
        {
            continue;
        }

        for (int i = 0; i < buf.vtxCount; i++)
            startMap[i] = -1;
        for (int i = 0; i < perimCount; i++)
        {
            int hi = perimHE[i];
            startMap[halfEdges[hi].a] = i;
        }

        for (int i = 0; i < perimCount; i++)
            used[i] = false;

        for (int start = 0; start < perimCount; start++)
        {
            if (used[start])
                continue;

            unsigned int loopCorners[64];
            int loopCount = 0;

            int curr = start;
            while (!used[curr])
            {
                used[curr] = true;
                int hi = perimHE[curr];
                loopCorners[loopCount++] = halfEdges[hi].a;

                unsigned int nextV = halfEdges[hi].b;
                int next = startMap[nextV];
                if (next == -1)
                    break;
                curr = next;
            }

            if (loopCount >= 3)
            {
                unsigned int pruned[64];
                int prunedCount = 0;
                for (int i = 0; i < loopCount; i++)
                {
                    int prev = (i + loopCount - 1) % loopCount;
                    int nxt = (i + 1) % loopCount;
                    Vec4 e1 = {
                        buf.vertices[vtxOffset + loopCorners[i]].x - buf.vertices[vtxOffset + loopCorners[prev]].x,
                        buf.vertices[vtxOffset + loopCorners[i]].y - buf.vertices[vtxOffset + loopCorners[prev]].y,
                        buf.vertices[vtxOffset + loopCorners[i]].z - buf.vertices[vtxOffset + loopCorners[prev]].z, 0};
                    Vec4 e2 = {
                        buf.vertices[vtxOffset + loopCorners[nxt]].x - buf.vertices[vtxOffset + loopCorners[i]].x,
                        buf.vertices[vtxOffset + loopCorners[nxt]].y - buf.vertices[vtxOffset + loopCorners[i]].y,
                        buf.vertices[vtxOffset + loopCorners[nxt]].z - buf.vertices[vtxOffset + loopCorners[i]].z, 0};
                    float len1 = vec4::length(e1);
                    float len2 = vec4::length(e2);
                    if (len1 < 1e-12f || len2 < 1e-12f)
                    {
                        pruned[prunedCount++] = loopCorners[i];
                        continue;
                    }
                    float dot = vec4::dot3(e1, e2) / (len1 * len2);
                    if (dot >= cosTol && (loopCount - prunedCount) > 3)
                    {
                        continue;
                    }
                    pruned[prunedCount++] = loopCorners[i];
                }

                if (prunedCount >= 3)
                {
                    NewFace &nf = regionNewFaces[rid];
                    nf.cornerCount = prunedCount;
                    for (int i = 0; i < prunedCount; i++)
                    {
                        nf.corners[i] = pruned[i];
                    }
                    nf.color = buf.faceBatch.faceColors[faceOffset + seedFace[rid]];
                    regionEmitted[rid] = true;
                }
            }
        }
    }

    static NewFace outFaces[face::MAX_FACES];
    int outFaceCount = 0;
    for (int i = 0; i < faceCount; i++)
    {
        if (!selected[i])
        {
            int cs = buf.faceBatch.faceCornerStarts[faceOffset + i];
            int cc = buf.faceBatch.faceCornerCounts[faceOffset + i];
            NewFace &of = outFaces[outFaceCount++];
            of.cornerCount = cc;
            for (int c = 0; c < cc; c++)
                of.corners[c] = buf.faceBatch.faceCorners[cs + c];
            of.color = buf.faceBatch.faceColors[faceOffset + i];
        }
        else
        {
            int rid = regionId[i];
            if (rid != -1 && regionEmitted[rid] && seedFace[rid] == i)
            {
                outFaces[outFaceCount++] = regionNewFaces[rid];
            }
        }
    }

    if (outFaceCount == 0)
    {
        return faceCount;
    }

    int runningCorner = buf.faceBatch.faceCornerStarts[faceOffset];
    for (int i = 0; i < outFaceCount; i++)
    {
        int f = faceOffset + i;
        buf.faceBatch.faceCornerStarts[f] = runningCorner;
        buf.faceBatch.faceCornerCounts[f] = outFaces[i].cornerCount;
        buf.faceBatch.faceColors[f] = outFaces[i].color;
        for (int c = 0; c < outFaces[i].cornerCount; c++)
        {
            buf.faceBatch.faceCorners[runningCorner + c] = outFaces[i].corners[c];
        }
        runningCorner += outFaces[i].cornerCount;
    }
    buf.faceBatch.faceCounts[ref] = outFaceCount;
    buf.faceBatch.faceCount = faceOffset + outFaceCount;
    buf.faceBatch.cornerCount = runningCorner;
    buf.meshDirty = true;

    printf("merged coplanar: %d faces -> %d faces\n", faceCount, outFaceCount);

    return outFaceCount;
}

} // namespace mesh
