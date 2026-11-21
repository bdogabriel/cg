#include "gl_util.h"

#include "action.h"
#include "draw_3d.h"
#include "face_edit.h"
#include "mesh_edit.h"
#include "scene.h"
#include "skybox.h"
#include "trs.h"
#include "wavefront.h"
#include <stdio.h>

namespace cmd
{

namespace
{

const char *const reset_choices[] = {"rotation", "translation", "scale", nullptr};
const char *const clear_choices[] = {"true", "false", nullptr};

} // namespace

const ParamSpec params_cycle[] = {
    {"step", ParamType::INT, nullptr, "+1 next, anything <=0 prev", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_reset[] = {
    {"property", ParamType::STRING, reset_choices, "which component to reset", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_add_pyramid[] = {
    {"sides", ParamType::INT, nullptr, "number of base sides", false, "4"},
    {"color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", false, "ffffff"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_add_cylinder[] = {
    {"segments", ParamType::INT, nullptr, "number of side segments", false, "8"},
    {"color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", false, "ffffff"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_add_polyhedron[] = {
    {"faces", ParamType::INT, nullptr, "face count (4, 6, 8, 12, 20)", false, "6"},
    {"color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", false, "ffffff"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_add_sphere[] = {
    {"segments", ParamType::INT, nullptr, "segments (slices and stacks)", false, "16"},
    {"color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", false, "ffffff"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_save_obj[] = {
    {"file", ParamType::STRING, nullptr, "output filename (default = current file)", false, ""},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_save_scene[] = {
    {"name", ParamType::STRING, nullptr, "scene name (default scene)", false, "scene"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_load_scene[] = {
    {"name", ParamType::STRING, nullptr, "scene name", true, nullptr},
    {"clear", ParamType::STRING, clear_choices, "clear existing scene first", false, "false"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_load_obj[] = {
    {"file", ParamType::STRING, nullptr, "input filename", true, nullptr},
    {"clear", ParamType::STRING, clear_choices, "clear existing scene first", false, "false"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_load_skybox[] = {
    {"file", ParamType::STRING, nullptr, "path to equirectangular skybox image", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_set_color[] = {
    {"color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_merge_coplanar[] = {
    {"angle", ParamType::INT, nullptr, "angle tolerance in degrees (default 1)", false, "1"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

void do_set_color(Editor &e, bool)
{
    Color c;
    if (!color::from_hex(param::get(e.args, "color"), c))
    {
        printf("E: invalid color '%s' (expected #RRGGBB or #RRGGBBAA)\n", param::get(e.args, "color"));
        return;
    }
    e.currentColor = c;
}

void do_apply_color(Editor &e, bool justPressed)
{
    if (!BUF(bool, e.meshBatch.usedSlots)[e.selectedRef])
    {
        return;
    }
    int base = BUF(int, e.meshBatch.faceBatch.faceOffsets)[e.selectedRef];
    Color c = e.currentColor;

    if (e.target == Target::FACE)
    {
        if (e.selectedFaceCount == 0)
        {
            printf("E: no faces selected\n");
            return;
        }
        if (justPressed)
            editor::push_undo(e);
        for (int i = 0; i < e.selectedFaceCount; i++)
        {
            BUF(Color, e.meshBatch.faceBatch.faceColors)[base + BUF(int, e.selectedFaces)[i]] = c;
        }
    }
    else
    {
        if (justPressed)
            editor::push_undo(e);
        int count = BUF(int, e.meshBatch.faceBatch.faceCounts)[e.selectedRef];
        for (int f = 0; f < count; f++)
        {
            BUF(Color, e.meshBatch.faceBatch.faceColors)[base + f] = c;
        }
    }
    e.meshBatch.meshDirty = true;
}

void do_cycle(Editor &e, bool)
{
    int step = param::get_int(e.args, "step");
    bool isFace = (e.target == Target::FACE);

    if (isFace)
    {
        int total = BUF(int, e.meshBatch.faceBatch.faceCounts)[e.selectedRef];
        if (total > 0)
        {
            e.faceCursor = (e.faceCursor + step + total) % total;
        }
        return;
    }

    if (step > 0)
    {
        for (int j = e.selectedRef + 1;; j++)
        {
            if (j >= e.meshBatch.slotCount)
            {
                j = 1;
            }
            if (j == e.selectedRef)
            {
                return;
            }
            if (BUF(bool, e.meshBatch.usedSlots)[j])
            {
                e.selectedRef = j;
                e.faceCursor = 0;
                e.selectedFaceCount = 0;
                return;
            }
        }
    }
    else
    {
        for (int j = e.selectedRef - 1;; j--)
        {
            if (j < 1)
            {
                j = e.meshBatch.slotCount - 1;
            }
            if (j == e.selectedRef)
            {
                return;
            }
            if (BUF(bool, e.meshBatch.usedSlots)[j])
            {
                e.selectedRef = j;
                e.faceCursor = 0;
                e.selectedFaceCount = 0;
                return;
            }
        }
    }
}

void do_reset(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::push_undo(e);
    }
    if (strcmp(param::get(e.args, "property"), "rotation") == 0)
    {
        BUF(TRS, e.transforms)[e.selectedRef].r = quat::identity();
    }
    else if (strcmp(param::get(e.args, "property"), "scale") == 0)
    {
        BUF(TRS, e.transforms)[e.selectedRef].sx = 1;
        BUF(TRS, e.transforms)[e.selectedRef].sy = 1;
        BUF(TRS, e.transforms)[e.selectedRef].sz = 1;
    }
    else
    {
        BUF(TRS, e.transforms)[e.selectedRef].tx = 0;
        BUF(TRS, e.transforms)[e.selectedRef].ty = 0;
        BUF(TRS, e.transforms)[e.selectedRef].tz = 0;
    }
    mesh::compose_model(e.meshBatch, e.selectedRef, trs::compose(BUF(TRS, e.transforms)[e.selectedRef]));
}

void do_undo(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::pop_undo(e);
    }
}

void do_extrude(Editor &e, bool justPressed)
{
    bool isFace = (e.target == Target::FACE);
    if (isFace && e.selectedFaceCount == 0)
    {
        return;
    }
    if (justPressed)
    {
        editor::push_undo(e);
    }
    faceedit::extrude(e.meshBatch, e.selectedRef, BUF(int, e.selectedFaces), e.selectedFaceCount);
    e.cmd = CommandId::camera;
}

namespace
{

void add_mesh(Editor &e, const Mesh &mesh, const char *colorStr, const char *name)
{
    if (!mesh::can_add(e.meshBatch, mesh))
    {
        return;
    }

    Mesh m = mesh;
    TRS t;
    t.sx = t.sy = t.sz = e.defaultMeshScale;
    m.model = trs::compose(t);
    Ref newRef = mesh::add(e.meshBatch, m);
    buffer::ensure(e.transforms, sizeof(TRS), e.meshBatch.slotCount);
    BUF(TRS, e.transforms)[newRef] = t;

    Color color;
    if (color::from_hex(colorStr, color))
    {
        int base = BUF(int, e.meshBatch.faceBatch.faceOffsets)[newRef];
        int count = BUF(int, e.meshBatch.faceBatch.faceCounts)[newRef];
        for (int f = 0; f < count; f++)
        {
            BUF(Color, e.meshBatch.faceBatch.faceColors)[base + f] = color;
        }
        e.meshBatch.meshDirty = true;
    }

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("added %s (object %d)\n", name, newRef);
}

} // namespace

void do_add_pyramid(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::pyramid(param::get_int(e.args, "sides")), param::get(e.args, "color"), "pyramid");
}

void do_add_cylinder(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::cylinder(param::get_int(e.args, "segments")), param::get(e.args, "color"), "cylinder");
}

void do_add_polyhedron(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::push_undo(e);
    }
    int faces = param::get_int(e.args, "faces");
    if (faces != 4 && faces != 6 && faces != 8 && faces != 12 && faces != 20)
    {
        printf("E: invalid face count %d (valid: 4, 6, 8, 12, 20)\n", faces);
        return;
    }
    add_mesh(e, draw3d::polyhedron(faces), param::get(e.args, "color"), "polyhedron");
}

void do_add_sphere(Editor &e, bool justPressed)
{
    if (justPressed)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::sphere(param::get_int(e.args, "segments")), param::get(e.args, "color"), "sphere");
}

void do_save_obj(Editor &e, bool)
{
    const char *file = param::get(e.args, "file");
    if (file[0] != '\0')
    {
        strncpy(e.currentFile, file, sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
    }
    else if (e.currentFile[0] == '\0')
    {
        strncpy(e.currentFile, "object.obj", sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
    }
    wfront::save(e.meshBatch, e.modelTextureCache, e.selectedRef, e.currentFile);
}

void do_save_scene(Editor &e, bool)
{
    const char *name = param::get(e.args, "name");
    scene::save(e, name);
    e.cmd = CommandId::camera;
}

void do_load_scene(Editor &e, bool)
{
    const char *name = param::get(e.args, "name");
    bool clear = (strcmp(param::get(e.args, "clear"), "true") == 0);
    if (scene::load(e, name, clear))
    {
        e.cmd = CommandId::camera;
        e.target = Target::OBJECT;
        e.locked = false;
    }
}

void do_load_obj(Editor &e, bool)
{
    const char *file = param::get(e.args, "file");
    bool clear = (strcmp(param::get(e.args, "clear"), "true") == 0);
    if (wfront::load(e.meshBatch, e, file, clear))
    {
        strncpy(e.currentFile, file, sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
        e.cmd = CommandId::camera;
        e.target = Target::OBJECT;
        e.locked = false;
    }
}

void do_load_skybox(Editor &e, bool)
{
    const char *file = param::get(e.args, "file");
    GLuint tex = glutil::load_texture(file, GL_RGBA8, GL_RGBA);
    if (tex)
    {
        skybox::set_texture(e.skybox, tex);
        strncpy(e.skyboxPath, file, sizeof(e.skyboxPath) - 1);
        e.skyboxPath[sizeof(e.skyboxPath) - 1] = '\0';
    }
}

void do_delete(Editor &e, bool justPressed)
{
    if (!BUF(bool, e.meshBatch.usedSlots)[e.selectedRef])
    {
        return;
    }
    if (justPressed)
    {
        editor::push_undo(e);
    }

    Ref oldRef = e.selectedRef;
    for (int j = oldRef + 1;; j++)
    {
        if (j >= e.meshBatch.slotCount)
        {
            j = 1;
        }
        if (j == oldRef)
        {
            break;
        }
        if (BUF(bool, e.meshBatch.usedSlots)[j])
        {
            e.selectedRef = j;
            break;
        }
    }

    mesh::remove(e.meshBatch, oldRef);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = CommandId::camera;
    e.meshBatch.meshDirty = true;
}

void do_merge(Editor &e, bool justPressed)
{
    int usedCount = 0;
    for (int j = 1; j < e.meshBatch.slotCount; j++)
    {
        if (BUF(bool, e.meshBatch.usedSlots)[j])
        {
            usedCount++;
        }
    }
    if (usedCount < 2)
    {
        return;
    }
    if (justPressed)
    {
        editor::push_undo(e);
    }

    int nextUsedSlot = e.selectedRef;
    for (int j = e.selectedRef + 1;; j++)
    {
        if (j >= e.meshBatch.slotCount)
        {
            j = 1;
        }
        if (BUF(bool, e.meshBatch.usedSlots)[j])
        {
            nextUsedSlot = j;
            break;
        }
    }

    e.selectedRef = meshedit::merge(e.meshBatch, e.selectedRef, nextUsedSlot);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = CommandId::camera;
}

void do_merge_coplanar(Editor &e, bool justPressed)
{
    if (!BUF(bool, e.meshBatch.usedSlots)[e.selectedRef])
    {
        return;
    }
    bool isFace = (e.target == Target::FACE);
    if (isFace && e.selectedFaceCount == 0)
    {
        printf("E: no faces selected\n");
        return;
    }
    if (justPressed)
    {
        editor::push_undo(e);
    }
    int angle = param::get_int(e.args, "angle");
    if (angle <= 0)
    {
        angle = 1;
    }
    if (isFace)
    {
        meshedit::merge_coplanar(e.meshBatch, e.selectedRef, (float)angle, BUF(int, e.selectedFaces),
                                 e.selectedFaceCount);
    }
    else
    {
        meshedit::merge_coplanar(e.meshBatch, e.selectedRef, (float)angle);
    }
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = CommandId::camera;
}

} // namespace cmd
