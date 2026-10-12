#include "ui_3d.h"
#include "arena.h"
#include "color.h"
#include "line_batch.h"
#include "trs.h"

namespace ui3d
{

void build_axes(const Editor &e, LineBatch &batch)
{
    Mat4 axisMat = mat4::IDENTITY;
    if (BUF(bool, e.meshBatch.usedSlots)[e.selectedRef])
    {
        TRS axisTRS = BUF(TRS, e.transforms)[e.selectedRef];
        axisTRS.sx = axisTRS.sy = axisTRS.sz = 0.8f;
        axisMat = trs::compose(axisTRS);
    }

    Vec4 o = axisMat * Vec4{0, 0, 0, 1};
    Vec4 x = axisMat * Vec4{1, 0, 0, 1};
    Vec4 y = axisMat * Vec4{0, 1, 0, 1};
    Vec4 z = axisMat * Vec4{0, 0, 1, 1};

    line::add(batch, o, x, color::palette[0]);
    line::add(batch, o, y, color::palette[1]);
    line::add(batch, o, z, color::palette[2]);
}

void build_face_highlights(const Editor &e, LineBatch &batch)
{
    if (e.target != Target::Face || !BUF(bool, e.meshBatch.usedSlots)[e.selectedRef])
    {
        return;
    }

    const MeshBatch &meshBatch = e.meshBatch;
    const DrawCommand &cmd = BUF(DrawCommand, meshBatch.drawCmds)[e.selectedRef];
    Mat4 model = BUF(Mat4, meshBatch.models)[e.selectedRef];

    auto add_outline = [&](int face, Color color) {
        int faceIdx = BUF(int, meshBatch.faceBatch.faceOffsets)[e.selectedRef] + face;
        int cornerStart = BUF(int, meshBatch.faceBatch.faceCornerStarts)[faceIdx];
        int cornerCount = BUF(int, meshBatch.faceBatch.faceCornerCounts)[faceIdx];
        Vec4 *corners = (Vec4 *)arena::scratch_alloc(sizeof(Vec4) * (size_t)cornerCount);
        for (int i = 0; i < cornerCount; i++)
        {
            corners[i] =
                model *
                BUF(Vec4, meshBatch.vertices)[cmd.vertexOffset +
                                              BUF(unsigned int, meshBatch.faceBatch.faceCorners)[cornerStart + i]];
        }
        for (int i = 0; i < cornerCount; i++)
        {
            line::add(batch, corners[i], corners[(i + 1) % cornerCount], color);
        }
    };

    add_outline(e.faceCursor, {255, 204, 0, 255});
    for (int i = 0; i < e.selectedFaceCount; i++)
    {
        add_outline(BUF(int, e.selectedFaces)[i], {255, 102, 0, 255});
    }
}

} // namespace ui3d
