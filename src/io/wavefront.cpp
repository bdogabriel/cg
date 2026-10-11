#include "wavefront.h"
#include "face_batch.h"
#include "file.h"
#include "mesh.h"
#include "model_texture_cache.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace wfront
{

void save(const MeshBatch &buf, const ModelTextureCache &modelTexCache, Ref ref, const char *path)
{
    const DrawCommand &cmd = buf.drawCmds[ref];
    bool textured = buf.texturedSlots[ref];

    int maxVtx = -1;
    int faceStart = buf.faceBatch.faceOffsets[ref];
    int faceEnd = faceStart + buf.faceBatch.faceCounts[ref];
    for (int f = faceStart; f < faceEnd; f++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[f];
        int cornerCount = buf.faceBatch.faceCornerCounts[f];
        for (int c = 0; c < cornerCount; c++)
        {
            int idx = buf.faceBatch.faceCorners[cornerStart + c];
            if (idx > maxVtx)
            {
                maxVtx = idx;
            }
        }
    }
    int vtxCount = maxVtx + 1;
    int faceCount = buf.faceBatch.faceCounts[ref];

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
        bool written[modeltexture::MAX_MODEL_TEXTURES] = {};
        for (int i = 0; i < faceCount; i++)
        {
            int mat = buf.faceBatch.faceMaterialIds[faceStart + i];
            if (mat >= 0 && !written[mat])
            {
                written[mat] = true;
                fprintf(mtl, "newmtl tex_%d\n", mat);
                fprintf(mtl, "Kd 1 1 1\n");
                fprintf(mtl, "map_Kd %s\n\n", modelTexCache.paths[mat]);
            }
        }
        // Flat materials for any untextured faces (no texture assigned).
        for (int i = 0; i < faceCount; i++)
        {
            int mat = buf.faceBatch.faceMaterialIds[faceStart + i];
            if (mat < 0)
            {
                const Color &col = buf.faceBatch.faceColors[faceStart + i];
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
            const Color &col = buf.faceBatch.faceColors[faceStart + i];
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
        const Vec4 &v = buf.vertices[cmd.vertexOffset + i];
        fprintf(f, "v %g %g %g\n", v.x, v.y, v.z);
    }
    if (textured)
    {
        for (int i = 0; i < vtxCount; i++)
        {
            const Vec2 &uv = buf.uvs[cmd.vertexOffset + i];
            fprintf(f, "vt %g %g\n", uv.x, 1.0f - uv.y);
        }
    }
    for (int i = 0; i < faceCount; i++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[faceStart + i];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceStart + i];
        if (textured)
        {
            int mat = buf.faceBatch.faceMaterialIds[faceStart + i];
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
            unsigned int idx = buf.faceBatch.faceCorners[cornerStart + c] + 1;
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

constexpr int BAKE_SIZE = 1 << 18; // 262144 slots
constexpr int MAX_MATERIALS = 64;

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

static Vec4 rawVerts[mesh::MAX_VERTICES];
static Vec2 rawUVs[mesh::MAX_VERTICES];
static BakeSlot bake[BAKE_SIZE];

static void dir_of(const char *path, char *out, size_t outSize)
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

static int find_material(const MtlDef *materials, int count, const char *name)
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

static int bake_lookup(int v, int vt, int *bakedCount, Vec4 *verts, Vec2 *uvs)
{
    unsigned int h = ((unsigned int)v * 104729u + (unsigned int)vt * 40507u) & (BAKE_SIZE - 1);
    for (int probe = 0; probe < BAKE_SIZE; probe++)
    {
        unsigned int slot = (h + (unsigned int)probe) & (BAKE_SIZE - 1);
        if (bake[slot].idx < 0)
        {
            int idx = *bakedCount;
            verts[idx] = rawVerts[v];
            uvs[idx] = rawUVs[vt];
            bake[slot] = {v, vt, idx};
            (*bakedCount)++;
            return idx;
        }
        if (bake[slot].v == v && bake[slot].vt == vt)
        {
            return bake[slot].idx;
        }
    }
    return -1; // table full
}

bool parse(const char *path, Vec4 *verts, Vec2 *uvs, int &vtxCount, unsigned int *faceCorners, int *faceCornerCounts,
           Color *faceColors, int *faceMaterialIds, int &faceCount, int &cornerCount, MtlDef *materials,
           int &materialCount)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("E: cannot open %s\n", path);
        return false;
    }

    memset(bake, 0xFF, sizeof(bake));

    int rawVtxCount = 0;
    int rawUvCount = 0;
    vtxCount = 0;
    faceCount = 0;
    cornerCount = 0;
    materialCount = 0;

    Color defaultColor = {255, 153, 51, 255};
    int currentMat = -1;

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
                        if (materialCount < MAX_MATERIALS)
                        {
                            cur = materialCount++;
                            MtlDef &m = materials[cur];
                            sscanf(mline + 7, "%127s", m.name);
                            m.kd = defaultColor;
                            m.mapKd[0] = '\0';
                            m.hasMap = false;
                        }
                        else
                        {
                            cur = -1;
                        }
                    }
                    else if (mline[0] == 'K' && mline[1] == 'd' && mline[2] == ' ' && cur >= 0)
                    {
                        float r, g, b;
                        if (sscanf(mline + 3, "%f %f %f", &r, &g, &b) == 3)
                        {
                            materials[cur].kd = color::from_float(r, g, b, materials[cur].kd.a / 255.0f);
                        }
                    }
                    else if (strncmp(mline, "map_Kd ", 7) == 0 && cur >= 0)
                    {
                        char val[256];
                        if (sscanf(mline + 7, "%255s", val) == 1)
                        {
                            if (val[0] == '/')
                            {
                                snprintf(materials[cur].mapKd, sizeof(materials[cur].mapKd), "%s", val);
                            }
                            else
                            {
                                char cwd[512];
                                if (getcwd(cwd, sizeof(cwd)))
                                {
                                    snprintf(materials[cur].mapKd, sizeof(materials[cur].mapKd), "%s/%s%s", cwd, mtlDir,
                                             val);
                                }
                                else
                                {
                                    snprintf(materials[cur].mapKd, sizeof(materials[cur].mapKd), "%s%s", mtlDir, val);
                                }
                            }
                            materials[cur].hasMap = true;
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
            currentMat = find_material(materials, materialCount, name);
        }
        else if (line[0] == 'v' && line[1] == ' ')
        {
            if (rawVtxCount >= mesh::MAX_VERTICES)
            {
                printf("E: too many vertices in %s\n", path);
                fclose(f);
                return false;
            }
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            rawVerts[rawVtxCount++] = {x, y, z, 1.0f};
        }
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ')
        {
            if (rawUvCount >= mesh::MAX_VERTICES)
            {
                printf("E: too many uvs in %s\n", path);
                fclose(f);
                return false;
            }
            float u, vv;
            sscanf(line + 3, "%f %f", &u, &vv);
            rawUVs[rawUvCount++] = {u, 1.0f - vv};
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            int cornerCountLocal = 0;
            unsigned int cornersV[32];
            unsigned int cornersVt[32];
            char *p = line + 2;
            while (*p && cornerCountLocal < 32)
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
                cornersV[cornerCountLocal] = (unsigned int)(vIdx > 0 ? vIdx - 1 : 0);
                cornersVt[cornerCountLocal] = (unsigned int)(vtIdx > 0 ? vtIdx - 1 : 0);
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
            if (faceCount >= face::MAX_FACES)
            {
                printf("E: too many faces in %s\n", path);
                fclose(f);
                return false;
            }
            if (cornerCount + cornerCountLocal > face::MAX_CORNERS)
            {
                printf("E: too many corners in %s\n", path);
                fclose(f);
                return false;
            }

            for (int i = 0; i < cornerCountLocal; i++)
            {
                int bi = bake_lookup((int)cornersV[i], (int)cornersVt[i], &vtxCount, verts, uvs);
                if (bi < 0)
                {
                    printf("E: vertex bake overflow in %s\n", path);
                    fclose(f);
                    return false;
                }
                faceCorners[cornerCount + i] = (unsigned int)bi;
            }
            faceCornerCounts[faceCount] = cornerCountLocal;
            faceColors[faceCount] = (currentMat >= 0) ? materials[currentMat].kd : defaultColor;
            faceMaterialIds[faceCount] = currentMat; // material def index, -1 if none
            faceCount++;
            cornerCount += cornerCountLocal;
        }
    }

    fclose(f);
    return true;
}
} // anonymous namespace

bool load(MeshBatch &buf, Editor &e, const char *path, bool clearBuffer)
{
    static Vec4 verts[mesh::MAX_VERTICES];
    static Vec2 uvs[mesh::MAX_VERTICES];
    int vtxCount;
    static unsigned int faceCorners[face::MAX_CORNERS];
    static int faceCornerCounts[face::MAX_FACES];
    static Color faceColors[face::MAX_FACES];
    static int faceMaterialIds[face::MAX_FACES];
    int faceCount;
    int cornerCount;

    MtlDef materials[MAX_MATERIALS];
    int materialCount;

    if (clearBuffer)
    {
        mesh::reset(buf);
        modeltexture::reset(e.modelTextureCache);
    }

    if (!parse(path, verts, uvs, vtxCount, faceCorners, faceCornerCounts, faceColors, faceMaterialIds, faceCount,
               cornerCount, materials, materialCount))
    {
        return false;
    }

    // resolve each material's texture to a texture index
    int defToHandle[MAX_MATERIALS];
    for (int i = 0; i < materialCount; i++)
    {
        defToHandle[i] = -1;
        if (materials[i].hasMap && materials[i].mapKd[0] != '\0')
        {
            defToHandle[i] = modeltexture::add(e.modelTextureCache, materials[i].mapKd);
        }
    }

    // remap per-face material-def indices to texture indices
    bool textured = false;
    for (int i = 0; i < faceCount; i++)
    {
        if (faceMaterialIds[i] >= 0)
        {
            faceMaterialIds[i] = defToHandle[faceMaterialIds[i]];
            if (faceMaterialIds[i] >= 0)
            {
                textured = true;
            }
        }
    }

    Mesh mesh{};
    mesh.vertices = verts;
    mesh.vtxCount = vtxCount;
    mesh.faceCorners = faceCorners;
    mesh.faceCornerCounts = faceCornerCounts;
    mesh.faceCount = faceCount;
    mesh.faceColors = faceColors;
    mesh.uvs = textured ? uvs : nullptr;
    mesh.faceMaterialIds = textured ? faceMaterialIds : nullptr;
    mesh.name = path;

    if (!mesh::can_add(buf, mesh))
    {
        return false;
    }

    Ref newRef = mesh::add(buf, mesh);
    e.transforms[newRef] = TRS{};

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("loaded %s\n", path);

    return true;
}
} // namespace wfront
