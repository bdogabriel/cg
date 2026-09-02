#include "file.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace file
{

char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "E: cannot open %s\n", path);
        return nullptr;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc((size_t)len + 1);
    if (!buf)
    {
        fprintf(stderr, "E: out of memory reading %s\n", path);
        fclose(f);
        return nullptr;
    }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

void replace_extension(const char *path, const char *ext, char *out, size_t outSize)
{
    const char *dot = strrchr(path, '.');
    size_t baseLen = dot ? (size_t)(dot - path) : strlen(path);
    snprintf(out, outSize, "%.*s.%s", (int)baseLen, path, ext);
}

const char *basename(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

bool mkdir_p(const char *path)
{
    char buf[512];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(buf))
    {
        return false;
    }
    memcpy(buf, path, len + 1);
    for (size_t i = 1; i <= len; i++)
    {
        if (buf[i] == '/' || buf[i] == '\0')
        {
            char saved = buf[i];
            buf[i] = '\0';
            if (mkdir(buf, 0755) != 0 && errno != EEXIST)
            {
                return false;
            }
            buf[i] = saved;
        }
    }
    return true;
}

} // namespace file
