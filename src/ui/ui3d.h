#ifndef UI3D_H
#define UI3D_H

#include "editor.h"
#include "linebatch.h"

namespace ui3d
{
void build_axes(const Editor &e, LineBatch &axes);
void build_face_highlights(const Editor &e, LineBatch &highlights);
}

#endif // UI3D_H
