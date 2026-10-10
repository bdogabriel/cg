#ifndef UI_2D_H
#define UI_2D_H

#include "editor.h"
#include "quad_batch.h"
#include "session.h"

namespace ui2d
{
void build_statusline(const Editor &e, const Session &s, int fbW, int fbH, QuadBatch &text, QuadBatch &solid);
void build_prompt(const Session &s, int fbW, int fbH, QuadBatch &text, QuadBatch &solid);
} // namespace ui2d

#endif
