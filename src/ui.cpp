#include "ui.h"
#include "mesh.h"

namespace ui
{

static void add_face_highlights(Buffer &buf, Buffer &highlight, Ref ref, int face, Color color, Mat4 model)
{
    DrawCommand &cmd = buf.drawCmds[ref];
    int base = cmd.indexOffset + face * 3;
    Vec4 v[3] = {
        buf.vertices[cmd.vertexOffset + buf.indices[base + 0]],
        buf.vertices[cmd.vertexOffset + buf.indices[base + 1]],
        buf.vertices[cmd.vertexOffset + buf.indices[base + 2]],
    };
    unsigned int idx[6] = {0, 1, 1, 2, 2, 0};
    Color c[3] = {color, color, color};
    buffer::add(highlight, Mesh{v, 3, idx, 6, c, 3, 2, model});
}

void build_overlays(const Editor &e, Buffer &highlights, Buffer &axes)
{
    // axes always rebuilt every frame
    buffer::reset(axes);
    Mat4 axisMat = mat4::IDENTITY;
    if (e.buffer.usedSlots[e.selectedRef])
    {
        TRS axisTRS = e.transforms[e.selectedRef];
        axisTRS.sx = axisTRS.sy = axisTRS.sz = 0.8f;
        axisMat = trs::compose(axisTRS);
    }
    Mesh axisX = mesh::make_axis('x');
    Mesh axisY = mesh::make_axis('y');
    Mesh axisZ = mesh::make_axis('z');
    axisX.model = axisMat;
    axisY.model = axisMat;
    axisZ.model = axisMat;
    buffer::add(axes, axisX);
    buffer::add(axes, axisY);
    buffer::add(axes, axisZ);

    // highlights: only in face-target mode with a valid selection
    buffer::reset(highlights);
    const char *target = e.args.get("target");
    if (target != nullptr && strcmp(target, "face") == 0 && e.buffer.usedSlots[e.selectedRef])
    {
        Mat4 model = e.buffer.models[e.selectedRef];
        // first arg is non-const in the signature but the body only reads it
        add_face_highlights(const_cast<Buffer &>(e.buffer), highlights, e.selectedRef, e.faceCursor,
                            {1.0f, 0.8f, 0.0f, 1}, model);
        for (int i = 0; i < e.selectedFaceCount; i++)
        {
            add_face_highlights(const_cast<Buffer &>(e.buffer), highlights, e.selectedRef, e.selectedFaces[i],
                                {1.0f, 0.4f, 0.0f, 1}, model);
        }
    }
}

} // namespace ui
