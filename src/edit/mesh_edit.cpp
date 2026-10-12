#include "mesh_edit.h"

#include "arena.h"

#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <string.h>

namespace meshedit
{

void move_to_end(MeshBatch &batch, Ref ref)
{
    DrawCommand &cmd = BUF(DrawCommand, batch.drawCmds)[ref];

    int origVtxOffset = cmd.vertexOffset;
    int origFaceOffset = BUF(int, batch.faceBatch.faceOffsets)[ref];
    int origFaceCount = BUF(int, batch.faceBatch.faceCounts)[ref];

    int maxLocalVtx = 0;
    for (int f = origFaceOffset; f < origFaceOffset + origFaceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[f];
        for (int c = 0; c < cornerCount; c++)
        {
            int idx = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + c];
            if (idx > maxLocalVtx)
            {
                maxLocalVtx = idx;
            }
        }
    }

    int nextVtxOffset = batch.vtxCount;
    for (int j = 1; j < batch.slotCount; j++)
    {
        if (!BUF(bool, batch.usedSlots)[j] || j == ref)
        {
            continue;
        }
        int jVtxOff = BUF(DrawCommand, batch.drawCmds)[j].vertexOffset;
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
        if (!BUF(bool, batch.usedSlots)[j] || j == ref)
        {
            continue;
        }
        int jFaceOff = BUF(int, batch.faceBatch.faceOffsets)[j];
        if (jFaceOff > origFaceOffset && jFaceOff < nextFaceOffset)
        {
            nextFaceOffset = jFaceOff;
        }
    }

    int refCornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[origFaceOffset];
    int nextCornerOffset = batch.faceBatch.cornerCount;
    if (origFaceOffset + origFaceCount < batch.faceBatch.faceCount)
    {
        nextCornerOffset = BUF(int, batch.faceBatch.faceCornerStarts)[origFaceOffset + origFaceCount];
    }
    int movedCornerCount = nextCornerOffset - refCornerStart;

    Vec4 *savedVerts = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)movedVtxCount);
    int *savedCornerCounts = (int *)arena::scratch_alloc(sizeof(int) * (size_t)origFaceCount);
    Color *savedColors = (Color *)arena::scratch_alloc(sizeof(Color) * (size_t)origFaceCount);
    unsigned int *savedCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)movedCornerCount);

    memcpy(savedVerts, BUF(Vec4, batch.vertices) + origVtxOffset, movedVtxCount * sizeof(Vec4));
    memcpy(savedCornerCounts, BUF(int, batch.faceBatch.faceCornerCounts) + origFaceOffset, origFaceCount * sizeof(int));
    memcpy(savedColors, BUF(Color, batch.faceBatch.faceColors) + origFaceOffset, origFaceCount * sizeof(Color));
    memcpy(savedCorners, BUF(unsigned int, batch.faceBatch.faceCorners) + refCornerStart,
           movedCornerCount * sizeof(unsigned int));

    memmove(BUF(Vec4, batch.vertices) + origVtxOffset, BUF(Vec4, batch.vertices) + origVtxOffset + movedVtxCount,
            (batch.vtxCount - origVtxOffset - movedVtxCount) * sizeof(Vec4));
    memmove(BUF(int, batch.faceBatch.faceCornerStarts) + origFaceOffset,
            BUF(int, batch.faceBatch.faceCornerStarts) + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(BUF(int, batch.faceBatch.faceCornerCounts) + origFaceOffset,
            BUF(int, batch.faceBatch.faceCornerCounts) + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(int));
    memmove(BUF(Color, batch.faceBatch.faceColors) + origFaceOffset,
            BUF(Color, batch.faceBatch.faceColors) + origFaceOffset + origFaceCount,
            (batch.faceBatch.faceCount - origFaceOffset - origFaceCount) * sizeof(Color));
    memmove(BUF(unsigned int, batch.faceBatch.faceCorners) + refCornerStart,
            BUF(unsigned int, batch.faceBatch.faceCorners) + refCornerStart + movedCornerCount,
            (batch.faceBatch.cornerCount - refCornerStart - movedCornerCount) * sizeof(unsigned int));

    int newVtxOffset = batch.vtxCount - movedVtxCount;
    int newFaceOffset = batch.faceBatch.faceCount - origFaceCount;
    int newCornerOffset = batch.faceBatch.cornerCount - movedCornerCount;

    memcpy(BUF(Vec4, batch.vertices) + newVtxOffset, savedVerts, movedVtxCount * sizeof(Vec4));
    memcpy(BUF(int, batch.faceBatch.faceCornerCounts) + newFaceOffset, savedCornerCounts, origFaceCount * sizeof(int));
    memcpy(BUF(Color, batch.faceBatch.faceColors) + newFaceOffset, savedColors, origFaceCount * sizeof(Color));
    memcpy(BUF(unsigned int, batch.faceBatch.faceCorners) + newCornerOffset, savedCorners,
           movedCornerCount * sizeof(unsigned int));

    for (int j = 1; j < batch.slotCount; j++)
    {
        if (!BUF(bool, batch.usedSlots)[j] || j == ref)
        {
            continue;
        }
        if (BUF(DrawCommand, batch.drawCmds)[j].vertexOffset > origVtxOffset)
        {
            BUF(DrawCommand, batch.drawCmds)[j].vertexOffset -= movedVtxCount;
        }
        if (BUF(int, batch.faceBatch.faceOffsets)[j] > origFaceOffset)
        {
            BUF(int, batch.faceBatch.faceOffsets)[j] -= origFaceCount;
        }
    }

    cmd.vertexOffset = newVtxOffset;
    BUF(int, batch.faceBatch.faceOffsets)[ref] = newFaceOffset;

    face::update_corner_starts(batch.faceBatch);
}

Ref merge(MeshBatch &batch, Ref dst, Ref src)
{
    move_to_end(batch, dst);
    move_to_end(batch, src);

    DrawCommand &dstCmd = BUF(DrawCommand, batch.drawCmds)[dst];
    DrawCommand &srcCmd = BUF(DrawCommand, batch.drawCmds)[src];

    int srcVtxStart = srcCmd.vertexOffset;
    int srcVtxCount = batch.vtxCount - srcVtxStart;
    int dstVtxStart = dstCmd.vertexOffset;

    Mat4 xform = mat4::inverse(BUF(Mat4, batch.models)[dst]) * BUF(Mat4, batch.models)[src];
    for (int i = 0; i < srcVtxCount; i++)
    {
        BUF(Vec4, batch.vertices)[srcVtxStart + i] = xform * BUF(Vec4, batch.vertices)[srcVtxStart + i];
    }

    int vtxRelOffset = srcVtxStart - dstVtxStart;
    int srcFaceOffset = BUF(int, batch.faceBatch.faceOffsets)[src];
    int srcFaceCount = BUF(int, batch.faceBatch.faceCounts)[src];

    for (int f = srcFaceOffset; f < srcFaceOffset + srcFaceCount; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[f];
        for (int c = 0; c < cornerCount; c++)
        {
            BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + c] += vtxRelOffset;
        }
    }

    BUF(int, batch.faceBatch.faceCounts)[dst] += srcFaceCount;

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

int he_compare(const HalfEdge *ha, const HalfEdge *hb)
{
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

bool he_same_canonical(const HalfEdge &a, const HalfEdge &b)
{
    unsigned int a_lo = a.a < a.b ? a.a : a.b;
    unsigned int a_hi = a.a < a.b ? a.b : a.a;
    unsigned int b_lo = b.a < b.b ? b.a : b.b;
    unsigned int b_hi = b.a < b.b ? b.b : b.a;
    return a_lo == b_lo && a_hi == b_hi;
}

struct NewFace
{
    unsigned int *corners; // points into scratch
    int cornerCount;
    Color color;
};

} // namespace

int merge_coplanar(MeshBatch &batch, Ref ref, float angleTolDeg, const int *faces, int selFaceCount)
{
    if (!BUF(bool, batch.usedSlots)[ref])
        return 0;

    int faceOffset = BUF(int, batch.faceBatch.faceOffsets)[ref];
    int faceCount = BUF(int, batch.faceBatch.faceCounts)[ref];
    if (faceCount <= 1)
        return faceCount;

    move_to_end(batch, ref);
    faceOffset = BUF(int, batch.faceBatch.faceOffsets)[ref];
    faceCount = BUF(int, batch.faceBatch.faceCounts)[ref];
    int vtxOffset = BUF(DrawCommand, batch.drawCmds)[ref].vertexOffset;

    bool *selected = (bool *)arena::scratch_alloc(sizeof(bool) * (size_t)faceCount);
    memset(selected, 0, sizeof(bool) * (size_t)faceCount);
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
        int cs = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cc = BUF(int, batch.faceBatch.faceCornerCounts)[f];
        for (int c = 0; c < cc; c++)
        {
            const Vec4 &v =
                BUF(Vec4, batch.vertices)[vtxOffset + BUF(unsigned int, batch.faceBatch.faceCorners)[cs + c]];
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

    Vec4 *normals = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)faceCount);
    float *offsets = (float *)arena::scratch_alloc(sizeof(float) * (size_t)faceCount);
    bool *degenerate = (bool *)arena::scratch_alloc(sizeof(bool) * (size_t)faceCount);

    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cc = BUF(int, batch.faceBatch.faceCornerCounts)[f];

        Vec4 *loop = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)cc);
        for (int c = 0; c < cc; c++)
        {
            loop[c] = BUF(Vec4, batch.vertices)[vtxOffset + BUF(unsigned int, batch.faceBatch.faceCorners)[cs + c]];
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
        totalHE += BUF(int, batch.faceBatch.faceCornerCounts)[faceOffset + i];
    }

    int *faceHEStart = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    HalfEdge *halfEdges = (HalfEdge *)arena::scratch_alloc(sizeof(HalfEdge) * (size_t)totalHE);
    int heIdx = 0;
    for (int i = 0; i < faceCount; i++)
    {
        int f = faceOffset + i;
        int cs = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cc = BUF(int, batch.faceBatch.faceCornerCounts)[f];
        faceHEStart[i] = heIdx;
        for (int c = 0; c < cc; c++)
        {
            unsigned int a = BUF(unsigned int, batch.faceBatch.faceCorners)[cs + c];
            unsigned int b = BUF(unsigned int, batch.faceBatch.faceCorners)[cs + (c + 1) % cc];
            halfEdges[heIdx++] = {a, b, i};
        }
    }

    int *order = (int *)arena::scratch_alloc(sizeof(int) * (size_t)totalHE);
    for (int i = 0; i < totalHE; i++)
        order[i] = i;
    std::sort(order, order + totalHE, [&](int ia, int ib) { return he_compare(&halfEdges[ia], &halfEdges[ib]) < 0; });

    int *reverse = (int *)arena::scratch_alloc(sizeof(int) * (size_t)totalHE);
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

    int *regionId = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    for (int i = 0; i < faceCount; i++)
        regionId[i] = -1;
    int regionCount = 0;

    Vec4 *seedNormals = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)faceCount);
    float *seedOffsets = (float *)arena::scratch_alloc(sizeof(float) * (size_t)faceCount);
    int *seedFace = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);
    int *queue = (int *)arena::scratch_alloc(sizeof(int) * (size_t)faceCount);

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
            int heCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceOffset + f];

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
                int pcs = BUF(int, batch.faceBatch.faceCornerStarts)[faceOffset + partner];
                int pcc = BUF(int, batch.faceBatch.faceCornerCounts)[faceOffset + partner];
                for (int c = 0; c < pcc; c++)
                {
                    const Vec4 &v =
                        BUF(Vec4, batch.vertices)[vtxOffset + BUF(unsigned int, batch.faceBatch.faceCorners)[pcs + c]];
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

    int *heRegion = (int *)arena::scratch_alloc(sizeof(int) * (size_t)totalHE);
    for (int i = 0; i < totalHE; i++)
    {
        heRegion[i] = regionId[halfEdges[i].face];
    }

    NewFace *regionNewFaces = (NewFace *)arena::scratch_alloc(sizeof(NewFace) * (size_t)faceCount);
    bool *regionEmitted = (bool *)arena::scratch_alloc(sizeof(bool) * (size_t)faceCount);
    for (int rid = 0; rid < regionCount; rid++)
        regionEmitted[rid] = false;

    int *perimHE = (int *)arena::scratch_alloc(sizeof(int) * (size_t)totalHE);
    int *startMap = (int *)arena::scratch_alloc(sizeof(int) * (size_t)batch.vtxCount);
    bool *used = (bool *)arena::scratch_alloc(sizeof(bool) * (size_t)totalHE);

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

            unsigned int *loopCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)perimCount);
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
                unsigned int *pruned = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)loopCount);
                int prunedCount = 0;
                for (int i = 0; i < loopCount; i++)
                {
                    int prev = (i + loopCount - 1) % loopCount;
                    int nxt = (i + 1) % loopCount;
                    Vec4 e1 = {BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].x -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[prev]].x,
                               BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].y -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[prev]].y,
                               BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].z -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[prev]].z,
                               0};
                    Vec4 e2 = {BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[nxt]].x -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].x,
                               BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[nxt]].y -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].y,
                               BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[nxt]].z -
                                   BUF(Vec4, batch.vertices)[vtxOffset + loopCorners[i]].z,
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
                    unsigned int *nfCorners =
                        (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)prunedCount);
                    nf.cornerCount = prunedCount;
                    for (int i = 0; i < prunedCount; i++)
                    {
                        nfCorners[i] = pruned[i];
                    }
                    nf.corners = nfCorners;
                    nf.color = BUF(Color, batch.faceBatch.faceColors)[faceOffset + seedFace[rid]];
                    regionEmitted[rid] = true;
                }
            }
        }
    }

    NewFace *outFaces = (NewFace *)arena::scratch_alloc(sizeof(NewFace) * (size_t)faceCount);
    int outFaceCount = 0;
    for (int i = 0; i < faceCount; i++)
    {
        if (!selected[i])
        {
            int cs = BUF(int, batch.faceBatch.faceCornerStarts)[faceOffset + i];
            int cc = BUF(int, batch.faceBatch.faceCornerCounts)[faceOffset + i];
            NewFace &of = outFaces[outFaceCount++];
            of.cornerCount = cc;
            unsigned int *ofCorners = (unsigned int *)arena::scratch_alloc(sizeof(unsigned int) * (size_t)cc);
            for (int c = 0; c < cc; c++)
                ofCorners[c] = BUF(unsigned int, batch.faceBatch.faceCorners)[cs + c];
            of.corners = ofCorners;
            of.color = BUF(Color, batch.faceBatch.faceColors)[faceOffset + i];
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

    int runningCorner = BUF(int, batch.faceBatch.faceCornerStarts)[faceOffset];
    for (int i = 0; i < outFaceCount; i++)
    {
        int f = faceOffset + i;
        BUF(int, batch.faceBatch.faceCornerStarts)[f] = runningCorner;
        BUF(int, batch.faceBatch.faceCornerCounts)[f] = outFaces[i].cornerCount;
        BUF(Color, batch.faceBatch.faceColors)[f] = outFaces[i].color;
        for (int c = 0; c < outFaces[i].cornerCount; c++)
        {
            BUF(unsigned int, batch.faceBatch.faceCorners)[runningCorner + c] = outFaces[i].corners[c];
        }
        runningCorner += outFaces[i].cornerCount;
    }
    BUF(int, batch.faceBatch.faceCounts)[ref] = outFaceCount;
    batch.faceBatch.faceCount = faceOffset + outFaceCount;
    batch.faceBatch.cornerCount = runningCorner;
    batch.meshDirty = true;

    printf("merged coplanar: %d faces -> %d faces\n", faceCount, outFaceCount);

    return outFaceCount;
}

} // namespace meshedit
