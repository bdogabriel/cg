#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include "editor.h"

namespace wfront
{
bool load(Buffer &buf, Editor &e, const char *path, bool clearBuffer);
void save(const Buffer &buf, Ref ref, const char *path);
} // namespace wfront

#endif
