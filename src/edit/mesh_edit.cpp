#include "mesh_edit.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace meshedit
{

void move_to_end(MeshBatch &batch, Ref ref)
{
    DrawCommand &cmd = batch.drawCmds[ref];

    int origVtxOffset = cmd.vertexOffset;
    int origFaceOffset = batch.faceBatch.faceOffsets[ref];
    int origFaceCount = batch.faceBatch.faceCounts[ref];

    int maxLocalVtx = 0;
    for (int f = origFaceOffset; f < origFaceOffset + origFaceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[f];
        int cornerCount = batch.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cornerCount; c++)
        {
            int idx = batch.faceBatch.faceCorners[cornerStart + c];
            if (idx > maxLocalVtx)
            {
                maxLocalVtx = idx;
            }
        }
    }

    int nextVtxOffset = batch.vtxCount;
    for (int j = 1; j < batch.slotCount; j++)
    {
        if (!batch.usedSlots[j] || j == ref)
        {
            continue;
        }
        int jVtxOff = batch.drawCmds[j].vertexOffset;
        if (jVtxOff > origVtxOffset && jVtxOff < nextVtxOffset)
        {
            nextVtxOffset = jVtxOff;
        }
    }

    int movedVtxCount = nextVtxOffset - origVtxOffset;
    if (movedVtxCount == 0 && origFaceOffset + origFaceCount == batch.faceBatch.faceCount)
    {
        return;
    }

    int nextFaceOffset = batch.faceBatch.faceCount;
    for (int j = 1; j < batch.slotCount; j++)
    {
        if (!batch.usedSlots[j] || j == ref)
        {
            continue;
        }
        int jFaceOff = batch.faceBatch.faceOffsets[j];
        if (jFaceOff > origFaceOffset && jFaceOff < nextFaceOffset)
        {
            nextFaceOffset = jFaceOff;
        }
    }

    int refCornerStart = batch.faceBatch.faceCornerStarts[origFaceOffset];
    int nextCornerOffset = batch.faceBatch.cornerCount;
    if (origFaceOffset + origFaceCount < batch.faceBatch.faceCount)
    {
        nextCornerOffset = batch.faceBatch.faceCornerStarts[origFaceOffset + origFaceCount];
    }
    int movedCornerCount = nextCornerOffset - refCornerStart;

    static Vec4 savedVerts[mesh::MAX_VERTICES];
    static int savedCornerCounts[face::MAX_FACES];
    static Color savedColors[face::MAX_FACES];
    static unsigned int savedCorners[face::MAX_CORNERS];

    memcpy(savedVerts, batch.vertices + origVtxOffset, movedVtxCount * sizeof(Vec4));
    memcpy(savedCornerCounts, batch.faceBatch.faceCornerCounts + origFaceOffset, origFaceCount * sizeof(int));
    memcpy(savedColors, batch.faceBatch.faceColors + origFaceOffset, origFaceCount * sizeof(Color));
    memcpy(savedCorners, batch.faceBatch.faceCorners + refCornerStart, movedCornerCount * sizeof(unsigned int));

    memmove(batch.vertices + origVtxOffset, batch.vertices + origVtxOffset + movedVtxCount,
            (batch.vtxCount - origVtxOffset - movedVtxCount) * sizeof(Vec4));
    memmove(batch.faceBatch.faceCornerStarts + origFaceOffset,
            batch.faceBatch.faceCornerStarts + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(batch.faceBatch.faceCornerCounts + origFaceOffset,
            batch.faceBatch.faceCornerCounts + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(batch.faceBatch.faceColors + origFaceOffset, batch.faceBatch.faceColors + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(Color));
    memmove(batch.faceBatch.faceCorners + refCornerStart,
            batch.faceBatch.faceCorners + refCornerStart + movedCornerCount,
            (batch.faceBatch.cornerCount - refCornerStart - movedCornerCount) * sizeof(unsigned int));

    int newVtxOffset = batch.vtxCount - movedVtxCount;
    int newFaceOffset = batch.faceBatch.faceCount - origFaceCount;
    int newCornerOffset = batch.faceBatch.cornerCount - movedCornerCount;

    memcpy(batch.vertices + newVtxOffset, savedVerts, movedVtxCount * sizeof(Vec4));
    memcpy(batch.faceBatch.faceCornerCounts + newFaceOffset, savedCornerCounts, origFaceCount * sizeof(int));
    memcpy(batch.faceBatch.faceColors + newFaceOffset, savedColors, origFaceCount * sizeof(Color));
    memcpy(batch.faceBatch.faceCorners + newCornerOffset, savedCorners, movedCornerCount * sizeof(unsigned int));

    for (int j = 1; j < batch.slotCount; j++)
    {
        if (!batch.usedSlots[j] || j == ref)
        {
            continue;
        }
        if (batch.drawCmds[j].vertexOffset > origVtxOffset)
        {
            batch.drawCmds[j].vertexOffset -= movedVtxCount;
        }
        if (batch.faceBatch.faceOffsets[j] > origFaceOffset)
        {
            batch.faceBatch.faceOffsets[j] -= origFaceCount;
        }
    }

    cmd.vertexOffset = newVtxOffset;
    batch.faceBatch.faceOffsets[ref] = newFaceOffset;

    face::update_corner_starts(batch.faceBatch);
}

Ref merge(MeshBatch &batch, Ref dst, Ref src)
{
    move_to_end(batch, dst);
    move_to_end(batch, src);

    DrawCommand &dstCmd = batch.drawCmds[dst];
    DrawCommand &srcCmd = batch.drawCmds[src];

    int srcVtxStart = srcCmd.vertexOffset;
    int srcVtxCount = batch.vtxCount - srcVtxStart;
    int dstVtxStart = dstCmd.vertexOffset;

    Mat4 xform = mat4::inverse(batch.models[dst]) * batch.models[src];
    for (int i = 0; i < srcVtxCount; i++)
    {
        batch.vertices[srcVtxStart + i] = xform * batch.vertices[srcVtxStart + i];
    }

    int vtxRelOffset = srcVtxStart - dstVtxStart;
    int srcFaceOffset = batch.faceBatch.faceOffsets[src];
    int srcFaceCount = batch.faceBatch.faceCounts[src];

    for (int f = srcFaceOffset; f < srcFaceOffset + srcFaceCount; f++)
    {
        int cornerStart = batch.faceBatch.faceCornerStarts[f];
        int cornerCount = batch.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cornerCount; c++)
        {
            batch.faceBatch.faceCorners[cornerStart + c] += vtxRelOffset;
        }
    }

    batch.faceBatch.faceCounts[dst] += srcFaceCount;

    mesh::remove(batch, src);

    batch.meshDirty = true;
    batch.modelsDirty = true;

    return dst;
}

namespace
{

Vec4 newell_normal(const Vec4 *verts, int count)
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

HalfEdge halfEdges[face::MAX_CORNERS];

int he_compare(const void *a, const void *b)
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

int he_order_compare(const void *a, const void *b)
{
    int ia = *(const int *)a, ib = *(const int *)b;
    return he_compare(&halfEdges[ia], &halfEdges[ib]);
}

bool he_same_canonical(const HalfEdge &a, const HalfEdge &b)
{
    unsigned int a_lo = a.a < a.b ? a.a : a.b;
    unsigned int a_hi = a.a < a.b ? a.b : a.a;
    unsigned int b_lo = b.a < b.b ? b.a : b.b;
    unsigned int b_hi = b.a < b.b ? b.b : b.a;
    return a_lo == b_lo && a_hi == b_hi;
}

} // namespace

int merge_coplanar(MeshBatch &batch, Ref ref, float angleTolDeg, const int *faces, int selFaceCount)
{
    if (!batch.usedSlots[ref])
        return 0;

    int faceOffset = batch.faceBatch.faceOffsets[ref];
    int faceCount = batch.faceBatch.faceCounts[ref];
    if (faceCount <= 1)
        return faceCount;

    move_to_end(batch, ref);
    faceOffset = batch.faceBatch.faceOffsets[ref];
    faceCount = batch.faceBatch.faceCounts[ref];
    int vtxOffset = batch.drawCmds[ref].vertexOffset;

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
        int cs = batch.faceBatch.faceCornerStarts[f];
        int cc = batch.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cc; c++)
        {
            const Vec4 &v = batch.vertices[vtxOffset + batch.faceBatch.faceCorners[cs + c]];
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
    float cosTol = cosf(angleTolDeg * mesh::PI / 180.0f);

    static Vec4 normals[face::MAX_FACES];
    static float offsets[face::MAX_FACES];
    static bool degenerate[face::MAX_FACES];

    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = batch.faceBatch.faceCornerStarts[f];
        int cc = batch.faceBatch.faceCornerCounts[f];

        Vec4 loop[64];
        for (int c = 0; c < cc; c++)
        {
            loop[c] = batch.vertices[vtxOffset + batch.faceBatch.faceCorners[cs + c]];
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
        totalHE += batch.faceBatch.faceCornerCounts[faceOffset + i];
    }

    int heIdx = 0;
    static int faceHEStart[face::MAX_FACES];
    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = batch.faceBatch.faceCornerStarts[f];
        int cc = batch.faceBatch.faceCornerCounts[f];
        faceHEStart[i] = heIdx;
        for (int c = 0; c < cc; c++)
        {
            unsigned int a = batch.faceBatch.faceCorners[cs + c];
            unsigned int b = batch.faceBatch.faceCorners[cs + (c + 1) % cc];
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
            int heCount = batch.faceBatch.faceCornerCounts[faceOffset + f];

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
                int pcs = batch.faceBatch.faceCornerStarts[faceOffset + partner];
                int pcc = batch.faceBatch.faceCornerCounts[faceOffset + partner];
                for (int c = 0; c < pcc; c++)
                {
                    const Vec4 &v = batch.vertices[vtxOffset + batch.faceBatch.faceCorners[pcs + c]];
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

        for (int i = 0; i < batch.vtxCount; i++)
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
                        batch.vertices[vtxOffset + loopCorners[i]].x - batch.vertices[vtxOffset + loopCorners[prev]].x,
                        batch.vertices[vtxOffset + loopCorners[i]].y - batch.vertices[vtxOffset + loopCorners[prev]].y,
                        batch.vertices[vtxOffset + loopCorners[i]].z - batch.vertices[vtxOffset + loopCorners[prev]].z,
                        0};
                    Vec4 e2 = {
                        batch.vertices[vtxOffset + loopCorners[nxt]].x - batch.vertices[vtxOffset + loopCorners[i]].x,
                        batch.vertices[vtxOffset + loopCorners[nxt]].y - batch.vertices[vtxOffset + loopCorners[i]].y,
                        batch.vertices[vtxOffset + loopCorners[nxt]].z - batch.vertices[vtxOffset + loopCorners[i]].z,
                        0};
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
                    nf.color = batch.faceBatch.faceColors[faceOffset + seedFace[rid]];
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
            int cs = batch.faceBatch.faceCornerStarts[faceOffset + i];
            int cc = batch.faceBatch.faceCornerCounts[faceOffset + i];
            NewFace &of = outFaces[outFaceCount++];
            of.cornerCount = cc;
            for (int c = 0; c < cc; c++)
                of.corners[c] = batch.faceBatch.faceCorners[cs + c];
            of.color = batch.faceBatch.faceColors[faceOffset + i];
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

    int runningCorner = batch.faceBatch.faceCornerStarts[faceOffset];
    for (int i = 0; i < outFaceCount; i++)
    {
        int f = faceOffset + i;
        batch.faceBatch.faceCornerStarts[f] = runningCorner;
        batch.faceBatch.faceCornerCounts[f] = outFaces[i].cornerCount;
        batch.faceBatch.faceColors[f] = outFaces[i].color;
        for (int c = 0; c < outFaces[i].cornerCount; c++)
        {
            batch.faceBatch.faceCorners[runningCorner + c] = outFaces[i].corners[c];
        }
        runningCorner += outFaces[i].cornerCount;
    }
    batch.faceBatch.faceCounts[ref] = outFaceCount;
    batch.faceBatch.faceCount = faceOffset + outFaceCount;
    batch.faceBatch.cornerCount = runningCorner;
    batch.meshDirty = true;

    printf("merged coplanar: %d faces -> %d faces\n", faceCount, outFaceCount);

    return outFaceCount;
}

} // namespace meshedit
