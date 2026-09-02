#include "ui3d.h"
#include "color.h"
#include "linebatch.h"
#include "trs.h"

namespace ui3d
{

void build_axes(const Editor &e, LineBatch &axes)
{
    Mat4 axisMat = mat4::IDENTITY;
    if (e.meshBatch.usedSlots[e.selectedRef])
    {
        TRS axisTRS = e.transforms[e.selectedRef];
        axisTRS.sx = axisTRS.sy = axisTRS.sz = 0.8f;
        axisMat = trs::compose(axisTRS);
    }

    Vec4 o = axisMat * Vec4{0, 0, 0, 1};
    Vec4 x = axisMat * Vec4{1, 0, 0, 1};
    Vec4 y = axisMat * Vec4{0, 1, 0, 1};
    Vec4 z = axisMat * Vec4{0, 0, 1, 1};

    line::add(axes, o, x, color::palette[0]);
    line::add(axes, o, y, color::palette[1]);
    line::add(axes, o, z, color::palette[2]);
}

void build_face_highlights(const Editor &e, LineBatch &highlights)
{
    if (e.target != Target::Face || !e.meshBatch.usedSlots[e.selectedRef])
    {
        return;
    }

    const MeshBatch &buf = e.meshBatch;
    const DrawCommand &cmd = buf.drawCmds[e.selectedRef];
    Mat4 model = buf.models[e.selectedRef];

    auto add_outline = [&](int face, Color color) {
        int faceIdx = buf.faceBatch.faceOffsets[e.selectedRef] + face;
        int cornerStart = buf.faceBatch.faceCornerStarts[faceIdx];
        int cornerCount = buf.faceBatch.faceCornerCounts[faceIdx];
        Vec4 corners[32];
        for (int i = 0; i < cornerCount; i++)
        {
            corners[i] = model * buf.vertices[cmd.vertexOffset + buf.faceBatch.faceCorners[cornerStart + i]];
        }
        for (int i = 0; i < cornerCount; i++)
        {
            line::add(highlights, corners[i], corners[(i + 1) % cornerCount], color);
        }
    };

    add_outline(e.faceCursor, {255, 204, 0, 255});
    for (int i = 0; i < e.selectedFaceCount; i++)
    {
        add_outline(e.selectedFaces[i], {255, 102, 0, 255});
    }
}

} // namespace ui3d
