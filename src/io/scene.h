#ifndef SCENE_H
#define SCENE_H

#include "editor.h"

// TODO: proper scene folder handling

namespace scene
{
bool save(Editor &e, const char *name);
bool load(Editor &e, const char *name, bool clear);
} // namespace scene

#endif
