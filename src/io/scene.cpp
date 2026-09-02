#include "file.h"
#include "wavefront.h"
#include <stdio.h>

namespace scene
{

static void get_dir(const char *path, char *out, size_t outSize)
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

bool save(Editor &e, const char *name)
{
    char dir[256];
    snprintf(dir, sizeof(dir), "scenes/%s", name);
    if (!file::mkdir_p(dir))
    {
        printf("E: cannot create directory %s\n", dir);
        return false;
    }

    char manifestPath[512];
    snprintf(manifestPath, sizeof(manifestPath), "%s/%s.scene", dir, name);

    FILE *f = fopen(manifestPath, "w");
    if (!f)
    {
        printf("E: cannot open %s for writing\n", manifestPath);
        return false;
    }

    fprintf(f, "scene 1\n");
    fprintf(f, "bg %u %u %u %u\n", e.bgColor.r, e.bgColor.g, e.bgColor.b, e.bgColor.a);

    int selOrdinal = -1;
    int ordinal = 0;
    for (int i = 1; i < e.meshBatch.slotCount; i++)
    {
        if (!e.meshBatch.usedSlots[i])
        {
            continue;
        }
        if (i == e.selectedRef)
        {
            selOrdinal = ordinal;
        }
        ordinal++;
    }
    fprintf(f, "selected %d\n", selOrdinal >= 0 ? selOrdinal : 0);

    for (int i = 1; i < e.meshBatch.slotCount; i++)
    {
        if (!e.meshBatch.usedSlots[i])
        {
            continue;
        }
        char objPath[512];
        snprintf(objPath, sizeof(objPath), "%s/%s_%d.obj", dir, name, i);
        wfront::save(e.meshBatch, i, objPath);
        const char *objBase = file::basename(objPath);
        const TRS &t = e.transforms[i];
        fprintf(f, "o %s\n", objBase);
        fprintf(f, "  t %g %g %g\n", t.tx, t.ty, t.tz);
        fprintf(f, "  r %g %g %g %g\n", t.r.x, t.r.y, t.r.z, t.r.w);
        fprintf(f, "  s %g %g %g\n", t.sx, t.sy, t.sz);
        fprintf(f, "  k %g %g %g %g %g %g\n", t.kxy, t.kxz, t.kyx, t.kyz, t.kzx, t.kzy);
    }

    fclose(f);
    printf("saved scene %s\n", manifestPath);
    return true;
}

bool load(Editor &e, const char *name, bool clear)
{
    char manifestPath[512];
    snprintf(manifestPath, sizeof(manifestPath), "scenes/%s/%s.scene", name, name);

    FILE *f = fopen(manifestPath, "r");
    if (!f)
    {
        printf("E: cannot open %s\n", manifestPath);
        return false;
    }

    if (clear)
    {
        mesh::reset(e.meshBatch);
        e.selectedRef = 0;
        e.faceCursor = 0;
        e.selectedFaceCount = 0;
    }

    char sceneDir[256] = {};
    get_dir(manifestPath, sceneDir, sizeof(sceneDir));

    char line[512];
    int savedBg[4] = {0, 0, 0, 255};
    int savedSel = 0;
    int loadedCount = 0;
    TRS curTRS = {};
    char curObj[256] = {};
    bool hasObj = false;

    auto flush = [&]() {
        if (!hasObj)
        {
            return;
        }
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s%s", sceneDir, curObj);
        if (wfront::load(e.meshBatch, e, fullPath, false))
        {
            Ref newRef = e.selectedRef;
            e.transforms[newRef] = curTRS;
            mesh::compose_model(e.meshBatch, newRef, trs::compose(curTRS));
            loadedCount++;
        }
        hasObj = false;
    };

    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "bg ", 3) == 0)
        {
            sscanf(line + 3, "%d %d %d %d", &savedBg[0], &savedBg[1], &savedBg[2], &savedBg[3]);
        }
        else if (strncmp(line, "selected ", 9) == 0)
        {
            sscanf(line + 9, "%d", &savedSel);
        }
        else if (line[0] == 'o' && line[1] == ' ')
        {
            flush();
            char *p = line + 2;
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            size_t len = strlen(p);
            while (len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r' || p[len - 1] == ' '))
            {
                len--;
            }
            if (len >= sizeof(curObj))
            {
                len = sizeof(curObj) - 1;
            }
            memcpy(curObj, p, len);
            curObj[len] = '\0';
            curTRS = {};
            hasObj = true;
        }
        else if (line[0] == ' ' || line[0] == '\t')
        {
            char *p = line;
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            if (p[0] == 't' && p[1] == ' ')
            {
                sscanf(p + 2, "%f %f %f", &curTRS.tx, &curTRS.ty, &curTRS.tz);
            }
            else if (p[0] == 'r' && p[1] == ' ')
            {
                sscanf(p + 2, "%f %f %f %f", &curTRS.r.x, &curTRS.r.y, &curTRS.r.z, &curTRS.r.w);
            }
            else if (p[0] == 's' && p[1] == ' ')
            {
                sscanf(p + 2, "%f %f %f", &curTRS.sx, &curTRS.sy, &curTRS.sz);
            }
            else if (p[0] == 'k' && p[1] == ' ')
            {
                sscanf(p + 2, "%f %f %f %f %f %f", &curTRS.kxy, &curTRS.kxz, &curTRS.kyx, &curTRS.kyz, &curTRS.kzx,
                       &curTRS.kzy);
            }
        }
    }
    flush();

    fclose(f);

    e.bgColor = {(uint8_t)savedBg[0], (uint8_t)savedBg[1], (uint8_t)savedBg[2], (uint8_t)savedBg[3]};

    int ordinal = 0;
    for (int i = 1; i < e.meshBatch.slotCount; i++)
    {
        if (!e.meshBatch.usedSlots[i])
        {
            continue;
        }
        if (ordinal == savedSel)
        {
            e.selectedRef = i;
            break;
        }
        ordinal++;
    }

    printf("loaded scene %s (%d objects)\n", manifestPath, loadedCount);
    return true;
}

} // namespace scene
