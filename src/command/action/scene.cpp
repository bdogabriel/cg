#include "scene.h"
#include "action.h"
#include "draw_3d.h"
#include "face_edit.h"
#include "mesh_edit.h"
#include "session.h"
#include "wavefront.h"
#include <stdio.h>

namespace cmd
{

static const char *const reset_choices[] = {"rotation", "translation", nullptr};
static const char *const clear_choices[] = {"true", "false", nullptr};

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

const ParamSpec params_save[] = {
    {"name", ParamType::STRING, nullptr, "scene name (default handmade)", false, "handmade"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_load[] = {
    {"name", ParamType::STRING, nullptr, "scene name", true, nullptr},
    {"clear", ParamType::STRING, clear_choices, "clear existing scene first", false, "false"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_load_obj[] = {
    {"file", ParamType::STRING, nullptr, "input filename", true, nullptr},
    {"clear", ParamType::STRING, clear_choices, "clear existing scene first", false, "false"},
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

void do_set_color(Editor &e, Session &, const Args &a, bool)
{
    Color c;
    if (!color::from_hex(a.get("color"), c))
    {
        printf("E: invalid color '%s' (expected #RRGGBB or #RRGGBBAA)\n", a.get("color"));
        return;
    }
    e.currentColor = c;
}

void do_apply_color(Editor &e, Session &, const Args &, bool snapshot)
{
    if (!e.meshBatch.usedSlots[e.selectedRef])
    {
        return;
    }
    int base = e.meshBatch.faceBatch.faceOffsets[e.selectedRef];
    Color c = e.currentColor;

    if (e.target == Target::Face)
    {
        if (e.selectedFaceCount == 0)
        {
            printf("E: no faces selected\n");
            return;
        }
        if (snapshot)
            editor::push_undo(e);
        for (int i = 0; i < e.selectedFaceCount; i++)
        {
            e.meshBatch.faceBatch.faceColors[base + e.selectedFaces[i]] = c;
        }
    }
    else
    {
        if (snapshot)
            editor::push_undo(e);
        int count = e.meshBatch.faceBatch.faceCounts[e.selectedRef];
        for (int f = 0; f < count; f++)
        {
            e.meshBatch.faceBatch.faceColors[base + f] = c;
        }
    }
    e.meshBatch.meshDirty = true;
}

void do_cycle(Editor &e, Session &s, const Args &a, bool)
{
    (void)s;
    int step = a.getInt("step");
    bool isFace = (e.target == Target::Face);

    if (isFace)
    {
        int total = e.meshBatch.faceBatch.faceCounts[e.selectedRef];
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
            if (e.meshBatch.usedSlots[j])
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
            if (e.meshBatch.usedSlots[j])
            {
                e.selectedRef = j;
                e.faceCursor = 0;
                e.selectedFaceCount = 0;
                return;
            }
        }
    }
}

void do_reset(Editor &e, Session &, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    if (strcmp(a.get("property"), "rotation") == 0)
    {
        e.transforms[e.selectedRef].r = quat::identity();
    }
    else
    {
        e.transforms[e.selectedRef].tx = 0;
        e.transforms[e.selectedRef].ty = 0;
        e.transforms[e.selectedRef].tz = 0;
    }
    mesh::compose_model(e.meshBatch, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
}

void do_undo(Editor &e, Session &, const Args &, bool snapshot)
{
    if (snapshot)
    {
        editor::pop_undo(e);
    }
}

void do_extrude(Editor &e, Session &s, const Args &, bool snapshot)
{
    (void)s;
    bool isFace = (e.target == Target::Face);
    if (isFace && e.selectedFaceCount == 0)
    {
        return;
    }
    if (snapshot)
    {
        editor::push_undo(e);
    }
    faceedit::extrude(e.meshBatch, e.selectedRef, e.selectedFaces, e.selectedFaceCount);
    s.cmd = get(CommandId::none);
}

static void add_mesh(Editor &e, const Mesh &mesh, const char *colorStr, const char *name)
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
    e.transforms[newRef] = t;

    Color color;
    if (color::from_hex(colorStr, color))
    {
        int base = e.meshBatch.faceBatch.faceOffsets[newRef];
        int count = e.meshBatch.faceBatch.faceCounts[newRef];
        for (int f = 0; f < count; f++)
        {
            e.meshBatch.faceBatch.faceColors[base + f] = color;
        }
        e.meshBatch.meshDirty = true;
    }

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("added %s (object %d)\n", name, newRef);
}

void do_add_pyramid(Editor &e, Session &, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::pyramid(a.getInt("sides")), a.get("color"), "pyramid");
}

void do_add_cylinder(Editor &e, Session &, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::cylinder(a.getInt("segments")), a.get("color"), "cylinder");
}

void do_add_polyhedron(Editor &e, Session &, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    int faces = a.getInt("faces");
    if (faces != 4 && faces != 6 && faces != 8 && faces != 12 && faces != 20)
    {
        printf("E: invalid face count %d (valid: 4, 6, 8, 12, 20)\n", faces);
        return;
    }
    add_mesh(e, draw3d::polyhedron(faces), a.get("color"), "polyhedron");
}

void do_add_sphere(Editor &e, Session &, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    add_mesh(e, draw3d::sphere(a.getInt("segments")), a.get("color"), "sphere");
}

void do_save_obj(Editor &e, Session &s, const Args &a, bool)
{
    const char *file = a.get("file");
    if (file[0] != '\0')
    {
        strncpy(s.currentFile, file, sizeof(s.currentFile) - 1);
        s.currentFile[sizeof(s.currentFile) - 1] = '\0';
    }
    else if (s.currentFile[0] == '\0')
    {
        strncpy(s.currentFile, "handmade.obj", sizeof(s.currentFile) - 1);
        s.currentFile[sizeof(s.currentFile) - 1] = '\0';
    }
    wfront::save(e.meshBatch, e.selectedRef, s.currentFile);
}

void do_save(Editor &e, Session &s, const Args &a, bool)
{
    const char *name = a.get("name");
    scene::save(e, name);
    s.cmd = get(CommandId::none);
}

void do_load(Editor &e, Session &s, const Args &a, bool)
{
    const char *name = a.get("name");
    bool clear = (strcmp(a.get("clear"), "true") == 0);
    if (scene::load(e, name, clear))
    {
        s.cmd = get(CommandId::none);
        e.target = Target::Object;
        e.locked = false;
    }
}

void do_load_obj(Editor &e, Session &s, const Args &a, bool)
{
    const char *file = a.get("file");
    bool clear = (strcmp(a.get("clear"), "true") == 0);
    if (wfront::load(e.meshBatch, e, file, clear))
    {
        strncpy(s.currentFile, file, sizeof(s.currentFile) - 1);
        s.currentFile[sizeof(s.currentFile) - 1] = '\0';
        s.cmd = get(CommandId::none);
        e.target = Target::Object;
        e.locked = false;
    }
}

void do_del(Editor &e, Session &s, const Args &, bool snapshot)
{
    (void)s;
    if (!e.meshBatch.usedSlots[e.selectedRef])
    {
        return;
    }
    if (snapshot)
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
        if (e.meshBatch.usedSlots[j])
        {
            e.selectedRef = j;
            break;
        }
    }

    mesh::remove(e.meshBatch, oldRef);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    s.cmd = get(CommandId::none);
    e.meshBatch.meshDirty = true;
}

void do_merge(Editor &e, Session &s, const Args &, bool snapshot)
{
    (void)s;
    int usedCount = 0;
    for (int j = 1; j < e.meshBatch.slotCount; j++)
    {
        if (e.meshBatch.usedSlots[j])
        {
            usedCount++;
        }
    }
    if (usedCount < 2)
    {
        return;
    }
    if (snapshot)
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
        if (e.meshBatch.usedSlots[j])
        {
            nextUsedSlot = j;
            break;
        }
    }

    e.selectedRef = meshedit::merge(e.meshBatch, e.selectedRef, nextUsedSlot);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    s.cmd = get(CommandId::none);
}

void do_merge_coplanar(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)s;
    if (!e.meshBatch.usedSlots[e.selectedRef])
    {
        return;
    }
    bool isFace = (e.target == Target::Face);
    if (isFace && e.selectedFaceCount == 0)
    {
        printf("E: no faces selected\n");
        return;
    }
    if (snapshot)
    {
        editor::push_undo(e);
    }
    int angle = a.getInt("angle");
    if (angle <= 0)
    {
        angle = 1;
    }
    if (isFace)
    {
        meshedit::merge_coplanar(e.meshBatch, e.selectedRef, (float)angle, e.selectedFaces, e.selectedFaceCount);
    }
    else
    {
        meshedit::merge_coplanar(e.meshBatch, e.selectedRef, (float)angle);
    }
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    s.cmd = get(CommandId::none);
}

} // namespace cmd
