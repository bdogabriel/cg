#ifndef FILE_H
#define FILE_H

#include <stddef.h>

namespace file
{
char *read_file(const char *path);
void replace_extension(const char *path, const char *ext, char *out, size_t outSize);
const char *basename(const char *path);
bool mkdir_p(const char *path);
} // namespace file

#endif
