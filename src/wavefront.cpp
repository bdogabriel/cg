#include "wavefront.h"

namespace wfront
{

static void mtl_path(const char *path, char *out, size_t outSize)
{
    const char *dot = strrchr(path, '.');
    size_t baseLen = dot ? (size_t)(dot - path) : strlen(path);
    snprintf(out, outSize, "%.*s.mtl", (int)baseLen, path);
}

static const char *path_basename(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

void save(const Buffer &buf, Ref ref, const char *path)
{
    const DrawCommand &cmd = buf.drawCmds[ref];

    int maxVtx = -1;
    for (unsigned int i = 0; i < cmd.indicesCount; i++)
    {
        int idx = (int)buf.indices[cmd.indexOffset + i];
        if (idx > maxVtx)
        {
            maxVtx = idx;
        }
    }
    int vtxCount = maxVtx + 1;
    int faceCount = cmd.indicesCount / 3;
    int faceOffset = buf.faceOffsets[ref];

    char mpath[256];
    mtl_path(path, mpath, sizeof(mpath));

    FILE *mtl = fopen(mpath, "w");
    if (!mtl)
    {
        printf("E: cannot open %s for writing\n", mpath);
        return;
    }
    for (int i = 0; i < faceCount; i++)
    {
        const Color &col = buf.faceColors[faceOffset + i];
        fprintf(mtl, "newmtl mat_%d\n", i);
        fprintf(mtl, "Kd %g %g %g\n", col.r, col.g, col.b);
        fprintf(mtl, "d %g\n\n", col.a);
    }
    fclose(mtl);

    FILE *f = fopen(path, "w");
    if (!f)
    {
        printf("E: cannot open %s for writing\n", path);
        return;
    }

    fprintf(f, "mtllib %s\no model\n", path_basename(mpath));
    const Mat4 &model = buf.models[ref];
    for (int i = 0; i < vtxCount; i++)
    {
        Vec4 w = model * buf.vertices[cmd.vertexOffset + i];
        fprintf(f, "v %g %g %g\n", w.x, w.y, w.z);
    }
    for (unsigned int i = 0; i < cmd.indicesCount; i += 3)
    {
        fprintf(f, "usemtl mat_%u\n", i / 3);
        fprintf(f, "f %u %u %u\n", buf.indices[cmd.indexOffset + i + 0] + 1, buf.indices[cmd.indexOffset + i + 1] + 1,
                buf.indices[cmd.indexOffset + i + 2] + 1);
    }
    fclose(f);

    printf("saved %s\n", path);
}

namespace
{
bool parse(const char *path, Vec4 *loadedVerts, int &loadedVtxCount, unsigned int *loadedIndices, int &loadedIdxCount,
           Color *loadedColors, int &loadedFaceCount)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("E: cannot open %s\n", path);
        return false;
    }

    Color matColors[MAX_INDICES / 3] = {};
    Color defaultColor = {1.0f, 0.6f, 0.2f, 1.0f};

    loadedVtxCount = 0;
    loadedIdxCount = 0;
    loadedFaceCount = 0;

    Color currentColor = defaultColor;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "mtllib ", 7) == 0)
        {
            char mpath[256];
            mtl_path(path, mpath, sizeof(mpath));
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
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            matColors[idx] = defaultColor;
                        }
                    }
                    else if (mline[0] == 'K' && mline[1] == 'd' && mline[2] == ' ')
                    {
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            sscanf(mline + 3, "%f %f %f", &matColors[idx].r, &matColors[idx].g, &matColors[idx].b);
                        }
                    }
                    else if (mline[0] == 'd' && mline[1] == ' ')
                    {
                        if (idx >= 0 && idx < MAX_INDICES / 3)
                        {
                            sscanf(mline + 2, "%f", &matColors[idx].a);
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
            if (idx >= 0 && idx < MAX_INDICES / 3)
            {
                currentColor = matColors[idx];
            }
        }
        else if (line[0] == 'v' && line[1] == ' ')
        {
            if (loadedVtxCount >= MAX_VERTICES)
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
            if (loadedIdxCount + 3 > MAX_INDICES)
            {
                printf("E: too many faces in %s\n", path);
                fclose(f);
                return false;
            }
            unsigned int a, b, c;
            sscanf(line + 2, "%u %u %u", &a, &b, &c);
            loadedIndices[loadedIdxCount++] = a - 1;
            loadedIndices[loadedIdxCount++] = b - 1;
            loadedIndices[loadedIdxCount++] = c - 1;
            loadedColors[loadedFaceCount++] = currentColor;
        }
    }

    fclose(f);
    return true;
}
} // anonymous namespace

bool load(Buffer &buf, Editor &e, const char *path, bool clearBuffer)
{
    Vec4 loadedVerts[MAX_VERTICES];
    int loadedVtxCount;
    unsigned int loadedIndices[MAX_INDICES];
    int loadedIdxCount;
    Color loadedColors[MAX_INDICES / 3];
    int loadedFaceCount;

    if (!parse(path, loadedVerts, loadedVtxCount, loadedIndices, loadedIdxCount, loadedColors, loadedFaceCount))
    {
        return false;
    }

    if (clearBuffer)
    {
        buffer::reset(buf);
    }

    if (!buffer::can_add(buf, Mesh{.vtxCount = loadedVtxCount, .idxCount = loadedIdxCount, .name = path}))
    {
        return false;
    }

    Ref newRef = buffer::add(buf, Mesh{loadedVerts, loadedVtxCount, loadedIndices, loadedIdxCount, loadedColors,
                                       loadedFaceCount, 3, mat4::IDENTITY, path});
    e.transforms[newRef] = TRS{};

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = &cmd::none;
    e.args.set("target", "object");
    e.args.clear("axis");
    e.args.clear("step");
    e.args.clear("lock");

    printf("loaded %s\n", path);

    return true;
}
} // namespace wfront
