#include "wavefront.h"
#include "face_batch.h"
#include "file.h"
#include "mesh.h"
#include <stdio.h>

namespace wfront
{

void save(const MeshBatch &buf, Ref ref, const char *path)
{
    const DrawCommand &cmd = buf.drawCmds[ref];

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
    for (int i = 0; i < faceCount; i++)
    {
        const Color &col = buf.faceBatch.faceColors[faceStart + i];
        fprintf(mtl, "newmtl mat_%d\n", i);
        fprintf(mtl, "Kd %g %g %g\n", col.r / 255.0f, col.g / 255.0f, col.b / 255.0f);
        fprintf(mtl, "d %g\n\n", col.a / 255.0f);
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
    for (int i = 0; i < faceCount; i++)
    {
        int cornerStart = buf.faceBatch.faceCornerStarts[faceStart + i];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceStart + i];
        fprintf(f, "usemtl mat_%d\n", i);
        fprintf(f, "f");
        for (int c = 0; c < cornerCount; c++)
        {
            fprintf(f, " %u", buf.faceBatch.faceCorners[cornerStart + c] + 1);
        }
        fprintf(f, "\n");
    }
    fclose(f);

    printf("saved %s\n", path);
}

namespace
{
bool parse(const char *path, Vec4 *loadedVerts, int &loadedVtxCount, unsigned int *loadedFaceCorners,
           int *loadedFaceCornerCounts, Color *loadedColors, int &loadedFaceCount, int &loadedCornerCount)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("E: cannot open %s\n", path);
        return false;
    }

    Color matColors[face::MAX_FACES] = {};
    Color defaultColor = {255, 153, 51, 255};

    loadedVtxCount = 0;
    loadedFaceCount = 0;
    loadedCornerCount = 0;

    Color currentColor = defaultColor;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "mtllib ", 7) == 0)
        {
            char mpath[256];
            file::replace_extension(path, "mtl", mpath, sizeof(mpath));
            FILE *mf = fopen(mpath, "r");
            if (mf)
            {
                int idx = -1;
                char mline[512];
                while (fgets(mline, sizeof(mline), mf))
                {
                    if (strncmp(mline, "newmtl ", 7) == 0)
                    {
                        sscanf(mline + 7, "mat_%d", &idx);
                        if (idx >= 0 && idx < face::MAX_FACES)
                        {
                            matColors[idx] = defaultColor;
                        }
                    }
                    else if (mline[0] == 'K' && mline[1] == 'd' && mline[2] == ' ')
                    {
                        if (idx >= 0 && idx < face::MAX_FACES)
                        {
                            float r, g, b;
                            sscanf(mline + 3, "%f %f %f", &r, &g, &b);
                            matColors[idx] = color::from_float(r, g, b, matColors[idx].a / 255.0f);
                        }
                    }
                    else if (mline[0] == 'd' && mline[1] == ' ')
                    {
                        if (idx >= 0 && idx < face::MAX_FACES)
                        {
                            float a;
                            sscanf(mline + 2, "%f", &a);
                            matColors[idx].a = static_cast<uint8_t>(a * 255.0f + 0.5f);
                        }
                    }
                }
                fclose(mf);
            }
        }
        else if (strncmp(line, "usemtl ", 7) == 0)
        {
            int idx = -1;
            sscanf(line + 7, "mat_%d", &idx);
            if (idx >= 0 && idx < face::MAX_FACES)
            {
                currentColor = matColors[idx];
            }
        }
        else if (line[0] == 'v' && line[1] == ' ')
        {
            if (loadedVtxCount >= mesh::MAX_VERTICES)
            {
                printf("E: too many vertices in %s\n", path);
                fclose(f);
                return false;
            }
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            loadedVerts[loadedVtxCount++] = {x, y, z, 1.0f};
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            int cornerCount = 0;
            unsigned int corners[32];
            char *p = line + 2;
            while (*p && cornerCount < 32)
            {
                while (*p == ' ' || *p == '\t')
                {
                    p++;
                }
                if (*p == '\0' || *p == '\n')
                {
                    break;
                }
                unsigned int idx;
                if (sscanf(p, "%u", &idx) != 1)
                {
                    break;
                }
                corners[cornerCount++] = idx - 1;
                while (*p && *p != ' ' && *p != '\t' && *p != '\n')
                {
                    p++;
                }
            }

            if (cornerCount < 3)
            {
                continue;
            }

            if (loadedFaceCount >= face::MAX_FACES)
            {
                printf("E: too many faces in %s\n", path);
                fclose(f);
                return false;
            }

            if (loadedCornerCount + cornerCount > face::MAX_CORNERS)
            {
                printf("E: too many corners in %s\n", path);
                fclose(f);
                return false;
            }

            for (int i = 0; i < cornerCount; i++)
            {
                loadedFaceCorners[loadedCornerCount + i] = corners[i];
            }
            loadedFaceCornerCounts[loadedFaceCount] = cornerCount;
            loadedColors[loadedFaceCount] = currentColor;
            loadedFaceCount++;
            loadedCornerCount += cornerCount;
        }
    }

    fclose(f);
    return true;
}
} // anonymous namespace

bool load(MeshBatch &buf, Editor &e, const char *path, bool clearBuffer)
{
    Vec4 loadedVerts[mesh::MAX_VERTICES];
    int loadedVtxCount;
    unsigned int loadedFaceCorners[face::MAX_CORNERS];
    int loadedFaceCornerCounts[face::MAX_FACES];
    Color loadedColors[face::MAX_FACES];
    int loadedFaceCount;
    int loadedCornerCount;

    if (!parse(path, loadedVerts, loadedVtxCount, loadedFaceCorners, loadedFaceCornerCounts, loadedColors,
               loadedFaceCount, loadedCornerCount))
    {
        return false;
    }

    if (clearBuffer)
    {
        mesh::reset(buf);
    }

    if (!mesh::can_add(buf, Mesh{.vertices = loadedVerts,
                                 .vtxCount = loadedVtxCount,
                                 .faceCorners = loadedFaceCorners,
                                 .faceCornerCounts = loadedFaceCornerCounts,
                                 .faceCount = loadedFaceCount,
                                 .faceColors = loadedColors,
                                 .name = path}))
    {
        return false;
    }

    Ref newRef = mesh::add(buf, Mesh{loadedVerts, loadedVtxCount, loadedFaceCorners, loadedFaceCornerCounts,
                                     loadedFaceCount, loadedColors, mat4::IDENTITY, path});
    e.transforms[newRef] = TRS{};

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("loaded %s\n", path);

    return true;
}
} // namespace wfront
