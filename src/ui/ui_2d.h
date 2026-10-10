#ifndef UI_2D_H
#define UI_2D_H

#include "editor.h"
#include "session.h"
#include "quad_batch.h"

namespace ui2d
{
void build_statusline(const Editor &e, const Session &s, int fbW, int fbH, QuadBatch &batch);
void build_prompt(const Session &s, int fbW, int fbH, QuadBatch &batch);
}

#endif
