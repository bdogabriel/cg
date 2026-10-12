#include "wavefront.h"
#include "arena.h"
#include "face_batch.h"
#include "file.h"
#include "mesh.h"
#include "model_texture_cache.h"
#include "trs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace wfront
{

void save(const MeshBatch &batch, const ModelTextureCache &modelTexCache, Ref ref, const char *path)
{
    const DrawCommand &cmd = BUF(DrawCommand, batch.drawCmds)[ref];
    bool textured = BUF(bool, batch.texturedSlots)[ref];

    int maxVtx = -1;
    int faceStart = BUF(int, batch.faceBatch.faceOffsets)[ref];
    int faceEnd = faceStart + BUF(int, batch.faceBatch.faceCounts)[ref];
    for (int f = faceStart; f < faceEnd; f++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[f];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[f];
        for (int c = 0; c < cornerCount; c++)
        {
            int idx = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + c];
            if (idx > maxVtx)
            {
                maxVtx = idx;
            }
        }
    }
    int vtxCount = maxVtx + 1;
    int faceCount = BUF(int, batch.faceBatch.faceCounts)[ref];

    char mpath[256];
    file::replace_extension(path, "mtl", mpath, sizeof(mpath));

    FILE *mtl = fopen(mpath, "w");
    if (!mtl)
    {
        printf("E: cannot open %s for writing\n", mpath);
        return;
    }

    if (textured)
    {
        // One material per distinct texture used by this object.
        int texCount = modeltexture::count(modelTexCache);
        bool *written = (bool *)arena::scratch_alloc(sizeof(bool) * (size_t)texCount);
        memset(written, 0, sizeof(bool) * (size_t)texCount);
        char objDir[512];
        file::dirname(path, objDir, sizeof(objDir));
        for (int i = 0; i < faceCount; i++)
        {
            int mat = BUF(int, batch.faceBatch.faceMaterialIds)[faceStart + i];
            if (mat >= 0 && !written[mat])
            {
                written[mat] = true;
                const char *src = BUF(TexturePath, modelTexCache.paths)[mat];
                char texName[512];
                snprintf(texName, sizeof(texName), "tex_%d_%s", mat, file::basename(src));
                char texDst[1024];
                snprintf(texDst, sizeof(texDst), "%s%s", objDir, texName);

                fprintf(mtl, "newmtl tex_%d\n", mat);
                fprintf(mtl, "Kd 1 1 1\n");
                if (file::copy(src, texDst))
                {
                    fprintf(mtl, "map_Kd %s\n\n", texName); // relative -> portable
                }
                else
                {
                    fprintf(mtl, "map_Kd %s\n\n", src); // fallback: original path
                }
            }
        }
        // Flat materials for any untextured faces (no texture assigned).
        for (int i = 0; i < faceCount; i++)
        {
            int mat = BUF(int, batch.faceBatch.faceMaterialIds)[faceStart + i];
            if (mat < 0)
            {
                const Color &col = BUF(Color, batch.faceBatch.faceColors)[faceStart + i];
                fprintf(mtl, "newmtl flat_%d\n", i);
                fprintf(mtl, "Kd %g %g %g\n", col.r / 255.0f, col.g / 255.0f, col.b / 255.0f);
                fprintf(mtl, "d %g\n\n", col.a / 255.0f);
            }
        }
    }
    else
    {
        for (int i = 0; i < faceCount; i++)
        {
            const Color &col = BUF(Color, batch.faceBatch.faceColors)[faceStart + i];
            fprintf(mtl, "newmtl mat_%d\n", i);
            fprintf(mtl, "Kd %g %g %g\n", col.r / 255.0f, col.g / 255.0f, col.b / 255.0f);
            fprintf(mtl, "d %g\n\n", col.a / 255.0f);
        }
    }
    fclose(mtl);

    FILE *f = fopen(path, "w");
    if (!f)
    {
        printf("E: cannot open %s for writing\n", path);
        return;
    }

    fprintf(f, "mtllib %s\no model\n", file::basename(mpath));
    for (int i = 0; i < vtxCount; i++)
    {
        const Vec4 &v = BUF(Vec4, batch.vertices)[cmd.vertexOffset + i];
        fprintf(f, "v %g %g %g\n", v.x, v.y, v.z);
    }
    if (textured)
    {
        for (int i = 0; i < vtxCount; i++)
        {
            const Vec2 &uv = BUF(Vec2, batch.uvs)[cmd.vertexOffset + i];
            fprintf(f, "vt %g %g\n", uv.x, 1.0f - uv.y);
        }
    }
    for (int i = 0; i < faceCount; i++)
    {
        int cornerStart = BUF(int, batch.faceBatch.faceCornerStarts)[faceStart + i];
        int cornerCount = BUF(int, batch.faceBatch.faceCornerCounts)[faceStart + i];
        if (textured)
        {
            int mat = BUF(int, batch.faceBatch.faceMaterialIds)[faceStart + i];
            if (mat >= 0)
            {
                fprintf(f, "usemtl tex_%d\n", mat);
            }
            else
            {
                fprintf(f, "usemtl flat_%d\n", i);
            }
        }
        else
        {
            fprintf(f, "usemtl mat_%d\n", i);
        }
        fprintf(f, "f");
        for (int c = 0; c < cornerCount; c++)
        {
            unsigned int idx = BUF(unsigned int, batch.faceBatch.faceCorners)[cornerStart + c] + 1;
            if (textured)
            {
                fprintf(f, " %u/%u", idx, idx);
            }
            else
            {
                fprintf(f, " %u", idx);
            }
        }
        fprintf(f, "\n");
    }
    fclose(f);

    printf("saved %s\n", path);
}

namespace
{

struct MtlDef
{
    char name[128];
    Color kd;
    char mapKd[1536];
    bool hasMap;
};

struct BakeSlot
{
    int v;
    int vt;
    int idx; // -1 = empty
};

// File-scope growable buffers reused across loads (capacity retained).
Buffer g_rawVerts;
Buffer g_rawUVs;
Buffer g_bake;
Buffer g_verts;
Buffer g_uvs;
Buffer g_faceCorners;
Buffer g_faceCornerCounts;
Buffer g_faceColors;
Buffer g_faceMaterialIds;
Buffer g_materials;

void dir_of(const char *path, char *out, size_t outSize)
{
    const char *slash = strrchr(path, '/');
    if (slash)
    {
        size_t len = (size_t)(slash - path + 1);
        if (len >= outSize)
        {
            len = outSize - 1;
        }
        memcpy(out, path, len);
        out[len] = '\0';
    }
    else
    {
        out[0] = '\0';
    }
}

int find_material(const MtlDef *materials, int count, const char *name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(materials[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void bake_reset()
{
    if (g_bake.cap == 0)
    {
        buffer::ensure(g_bake, sizeof(BakeSlot), 256);
    }
    BakeSlot *s = BUF(BakeSlot, g_bake);
    for (int i = 0; i < g_bake.cap; i++)
        s[i].idx = -1;
}

int bake_lookup(int v, int vt, int *bakedCount)
{
    if (g_bake.cap == 0)
    {
        bake_reset();
    }

    int cap = g_bake.cap;
    // Grow (rehash) when load factor would exceed ~0.7.
    if ((*bakedCount + 1) * 10 >= cap * 7)
    {
        int oldCap = cap;
        BakeSlot *old = (BakeSlot *)arena::scratch_alloc(sizeof(BakeSlot) * (size_t)oldCap);
        memcpy(old, BUF(BakeSlot, g_bake), sizeof(BakeSlot) * (size_t)oldCap);
        int newCap = oldCap * 2;
        buffer::ensure(g_bake, sizeof(BakeSlot), newCap);
        BakeSlot *ns = BUF(BakeSlot, g_bake);
        for (int i = 0; i < newCap; i++)
            ns[i].idx = -1;
        for (int i = 0; i < oldCap; i++)
        {
            if (old[i].idx < 0)
                continue;
            unsigned int h = ((unsigned int)old[i].v * 104729u + (unsigned int)old[i].vt * 40507u) & (newCap - 1);
            for (int probe = 0; probe < newCap; probe++)
            {
                unsigned int slot = (h + (unsigned int)probe) & (newCap - 1);
                if (ns[slot].idx < 0)
                {
                    ns[slot] = old[i];
                    break;
                }
            }
        }
        cap = newCap;
    }

    unsigned int h = ((unsigned int)v * 104729u + (unsigned int)vt * 40507u) & (cap - 1);
    for (int probe = 0; probe < cap; probe++)
    {
        unsigned int slot = (h + (unsigned int)probe) & (cap - 1);
        BakeSlot *bs = &BUF(BakeSlot, g_bake)[slot];
        if (bs->idx < 0)
        {
            int idx = *bakedCount;
            buffer::ensure(g_verts, sizeof(Vec4), idx + 1);
            buffer::ensure(g_uvs, sizeof(Vec2), idx + 1);
            BUF(Vec4, g_verts)[idx] = BUF(Vec4, g_rawVerts)[v];
            BUF(Vec2, g_uvs)[idx] = BUF(Vec2, g_rawUVs)[vt];
            bs->v = v;
            bs->vt = vt;
            bs->idx = idx;
            (*bakedCount)++;
            return idx;
        }
        if (bs->v == v && bs->vt == vt)
        {
            return bs->idx;
        }
    }
    return -1; // table full (should not happen under the load-factor threshold)
}

bool parse(const char *path, int &vtxCount, int &faceCount, int &cornerCount, int &materialCount)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("E: cannot open %s\n", path);
        return false;
    }

    bake_reset();

    // Ensure a valid element 0 (used when v/vt indices are absent).
    buffer::ensure(g_rawVerts, sizeof(Vec4), 1);
    BUF(Vec4, g_rawVerts)[0] = {0, 0, 0, 1};
    buffer::ensure(g_rawUVs, sizeof(Vec2), 1);
    BUF(Vec2, g_rawUVs)[0] = {0, 0};

    int rawVtxCount = 0;
    int rawUvCount = 0;
    vtxCount = 0;
    faceCount = 0;
    cornerCount = 0;
    materialCount = 0;

    Color defaultColor = {255, 153, 51, 255};
    int currentMat = -1;

    Buffer cornerV;
    Buffer cornerVt;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "mtllib ", 7) == 0)
        {
            char mtlName[256];
            if (sscanf(line + 7, "%255s", mtlName) != 1)
            {
                continue;
            }
            char objDir[512];
            dir_of(path, objDir, sizeof(objDir));
            char mpath[768];
            snprintf(mpath, sizeof(mpath), "%s%s", objDir, mtlName);
            char mtlDir[512];
            dir_of(mpath, mtlDir, sizeof(mtlDir));
            FILE *mf = fopen(mpath, "r");
            if (!mf)
            {
                printf("E: cannot open material library %s\n", mpath);
            }
            else
            {
                int cur = -1;
                char mline[512];
                while (fgets(mline, sizeof(mline), mf))
                {
                    if (strncmp(mline, "newmtl ", 7) == 0)
                    {
                        buffer::ensure(g_materials, sizeof(MtlDef), materialCount + 1);
                        cur = materialCount++;
                        MtlDef &m = BUF(MtlDef, g_materials)[cur];
                        sscanf(mline + 7, "%127s", m.name);
                        m.kd = defaultColor;
                        m.mapKd[0] = '\0';
                        m.hasMap = false;
                    }
                    else if (mline[0] == 'K' && mline[1] == 'd' && mline[2] == ' ' && cur >= 0)
                    {
                        float r, g, b;
                        if (sscanf(mline + 3, "%f %f %f", &r, &g, &b) == 3)
                        {
                            BUF(MtlDef, g_materials)
                            [cur].kd = color::from_float(r, g, b, BUF(MtlDef, g_materials)[cur].kd.a / 255.0f);
                        }
                    }
                    else if (strncmp(mline, "map_Kd ", 7) == 0 && cur >= 0)
                    {
                        char val[256];
                        if (sscanf(mline + 7, "%255s", val) == 1)
                        {
                            if (val[0] == '/')
                            {
                                snprintf(BUF(MtlDef, g_materials)[cur].mapKd,
                                         sizeof(BUF(MtlDef, g_materials)[cur].mapKd), "%s", val);
                            }
                            else
                            {
                                char cwd[512];
                                if (getcwd(cwd, sizeof(cwd)))
                                {
                                    snprintf(BUF(MtlDef, g_materials)[cur].mapKd,
                                             sizeof(BUF(MtlDef, g_materials)[cur].mapKd), "%s/%s%s", cwd, mtlDir, val);
                                }
                                else
                                {
                                    snprintf(BUF(MtlDef, g_materials)[cur].mapKd,
                                             sizeof(BUF(MtlDef, g_materials)[cur].mapKd), "%s%s", mtlDir, val);
                                }
                            }
                            BUF(MtlDef, g_materials)[cur].hasMap = true;
                        }
                    }
                }
                fclose(mf);
            }
        }
        else if (strncmp(line, "usemtl ", 7) == 0)
        {
            char name[128];
            sscanf(line + 7, "%127s", name);
            currentMat = find_material(BUF(MtlDef, g_materials), materialCount, name);
        }
        else if (line[0] == 'v' && line[1] == ' ')
        {
            buffer::ensure(g_rawVerts, sizeof(Vec4), rawVtxCount + 1);
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            BUF(Vec4, g_rawVerts)[rawVtxCount++] = {x, y, z, 1.0f};
        }
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ')
        {
            buffer::ensure(g_rawUVs, sizeof(Vec2), rawUvCount + 1);
            float u, vv;
            sscanf(line + 3, "%f %f", &u, &vv);
            BUF(Vec2, g_rawUVs)[rawUvCount++] = {u, 1.0f - vv};
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            int cornerCountLocal = 0;
            char *p = line + 2;
            while (*p)
            {
                while (*p == ' ' || *p == '\t')
                {
                    p++;
                }
                if (*p == '\0' || *p == '\n')
                {
                    break;
                }
                int vIdx = 0, vtIdx = -1;
                if (sscanf(p, "%d/%d", &vIdx, &vtIdx) < 1)
                {
                    break;
                }
                buffer::ensure(cornerV, sizeof(unsigned int), cornerCountLocal + 1);
                buffer::ensure(cornerVt, sizeof(unsigned int), cornerCountLocal + 1);
                BUF(unsigned int, cornerV)[cornerCountLocal] = (unsigned int)(vIdx > 0 ? vIdx - 1 : 0);
                BUF(unsigned int, cornerVt)[cornerCountLocal] = (unsigned int)(vtIdx > 0 ? vtIdx - 1 : 0);
                cornerCountLocal++;
                while (*p && *p != ' ' && *p != '\t' && *p != '\n')
                {
                    p++;
                }
            }

            if (cornerCountLocal < 3)
            {
                continue;
            }

            buffer::ensure(g_faceCorners, sizeof(unsigned int), cornerCount + cornerCountLocal);
            buffer::ensure(g_faceCornerCounts, sizeof(int), faceCount + 1);
            buffer::ensure(g_faceColors, sizeof(Color), faceCount + 1);
            buffer::ensure(g_faceMaterialIds, sizeof(int), faceCount + 1);

            for (int i = 0; i < cornerCountLocal; i++)
            {
                int bi =
                    bake_lookup((int)BUF(unsigned int, cornerV)[i], (int)BUF(unsigned int, cornerVt)[i], &vtxCount);
                if (bi < 0)
                {
                    printf("E: vertex bake overflow in %s\n", path);
                    fclose(f);
                    return false;
                }
                BUF(unsigned int, g_faceCorners)[cornerCount + i] = (unsigned int)bi;
            }
            BUF(int, g_faceCornerCounts)[faceCount] = cornerCountLocal;
            BUF(Color, g_faceColors)
            [faceCount] = (currentMat >= 0) ? BUF(MtlDef, g_materials)[currentMat].kd : defaultColor;
            BUF(int, g_faceMaterialIds)[faceCount] = currentMat; // material def index, -1 if none
            faceCount++;
            cornerCount += cornerCountLocal;
        }
    }

    fclose(f);
    return true;
}
} // namespace

bool load(MeshBatch &batch, Editor &e, const char *path, bool clearBuffer)
{
    int vtxCount;
    int faceCount;
    int cornerCount;
    int materialCount;

    if (clearBuffer)
    {
        mesh::reset(batch);
        modeltexture::reset(e.modelTextureCache);
    }

    if (!parse(path, vtxCount, faceCount, cornerCount, materialCount))
    {
        return false;
    }

    // Resolve each material's texture to a texture index.
    int *defToHandle = (int *)arena::scratch_alloc(sizeof(int) * (size_t)materialCount);
    for (int i = 0; i < materialCount; i++)
    {
        defToHandle[i] = -1;
        MtlDef &m = BUF(MtlDef, g_materials)[i];
        if (m.hasMap && m.mapKd[0] != '\0')
        {
            defToHandle[i] = modeltexture::add(e.modelTextureCache, m.mapKd);
        }
    }

    // Remap per-face material-def indices to texture indices.
    bool textured = false;
    for (int i = 0; i < faceCount; i++)
    {
        int mid = BUF(int, g_faceMaterialIds)[i];
        if (mid >= 0)
        {
            int th = defToHandle[mid];
            BUF(int, g_faceMaterialIds)[i] = th;
            if (th >= 0)
            {
                textured = true;
            }
        }
    }

    Mesh mesh{};
    mesh.vertices = BUF(Vec4, g_verts);
    mesh.vtxCount = vtxCount;
    mesh.faceCorners = BUF(unsigned int, g_faceCorners);
    mesh.faceCornerCounts = BUF(int, g_faceCornerCounts);
    mesh.faceCount = faceCount;
    mesh.faceColors = BUF(Color, g_faceColors);
    mesh.uvs = textured ? BUF(Vec2, g_uvs) : nullptr;
    mesh.faceMaterialIds = textured ? BUF(int, g_faceMaterialIds) : nullptr;
    mesh.name = path;

    if (!mesh::can_add(batch, mesh))
    {
        return false;
    }

    Ref newRef = mesh::add(batch, mesh);
    buffer::ensure(e.transforms, sizeof(TRS), e.meshBatch.slotCount);
    BUF(TRS, e.transforms)[newRef] = TRS{};

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("loaded %s\n", path);

    return true;
}
} // namespace wfront
