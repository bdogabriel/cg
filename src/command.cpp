#include "wavefront.h"
#include "mesh_edit.h"

namespace cmd
{

static void do_none(Editor &, const Args &, bool)
{
}

static const char *const cmd_choices[] = {"none",    "translate", "rotate", "scale", "shear",
                                          "extrude", "delete",    "merge",  nullptr};
static const char *const target_choices[] = {"object", "face", nullptr};
static const char *const axis_choices[] = {"x", "y", "z", nullptr};
static const char *const reset_choices[] = {"rotation", "translation", nullptr};
static const char *const shape_choices[] = {"cube", "pyramid", "cylinder", nullptr};
static const char *const clear_choices[] = {"true", "false", nullptr};

static const ParamSpec params_set_cmd[] = {
    {"op", ParamType::STRING, cmd_choices, "command to switch into", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_set_target[] = {
    {"t", ParamType::STRING, target_choices, "object or face target", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_set_lock[] = {
    {"axis", ParamType::STRING, axis_choices, "axis to lock/unlock (toggle)", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_set_args[] = {
    {"axis", ParamType::STRING, axis_choices, "axis to act on", true, nullptr},
    {"step", ParamType::INT, nullptr, "signed step size (negative = reverse)", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_cycle[] = {
    {"step", ParamType::INT, nullptr, "+1 next, anything <=0 prev", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_reset[] = {
    {"property", ParamType::STRING, reset_choices, "which component to reset", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_add_shape[] = {
    {"name", ParamType::STRING, shape_choices, "primitive shape", true, nullptr},
    {"param", ParamType::INT, nullptr, "shape parameter (sides/segments), -1 = default", false, "-1"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_save[] = {
    {"file", ParamType::STRING, nullptr, "output filename (default = current file)", false, ""},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_load[] = {
    {"file", ParamType::STRING, nullptr, "input filename", true, nullptr},
    {"clear", ParamType::STRING, clear_choices, "clear existing scene first", false, "false"},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static const ParamSpec params_help[] = {
    {"name", ParamType::STRING, nullptr, "command name (omit to list all)", false, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

// --- Setters ---------------------------------------------------------------

static void do_set_cmd(Editor &e, const Args &a, bool)
{
    const char *op = a.get("op");
    e.cmd = find(op);
    e.args.clear("axis");
    e.args.clear("step");
    if (strcmp(op, "extrude") == 0)
    {
        e.args.set("target", "face");
    }
}

static void do_set_args(Editor &e, const Args &a, bool)
{
    e.args.set("axis", a.get("axis"));
    e.args.set("step", a.get("step"));
}

static void do_set_target(Editor &e, const Args &a, bool)
{
    e.args.set("target", a.get("t"));
}

static void do_set_lock(Editor &e, const Args &a, bool)
{
    const char *axis = a.get("axis");
    const char *cur = e.args.get("lock");
    if (cur && strcmp(cur, axis) == 0)
    {
        e.args.clear("lock");
    }
    else
    {
        e.args.set("lock", axis);
    }
}

static void do_toggle_selection(Editor &e, const Args &, bool)
{
    const char *target = e.args.get("target");
    if (target && strcmp(target, "object") == 0)
    {
        return;
    }
    for (int i = 0; i < e.selectedFaceCount; i++)
    {
        if (e.selectedFaces[i] == e.faceCursor)
        {
            e.selectedFaces[i] = e.selectedFaces[e.selectedFaceCount - 1];
            e.selectedFaceCount--;
            return;
        }
    }
    e.selectedFaces[e.selectedFaceCount++] = e.faceCursor;
}

static void do_clear_selection(Editor &e, const Args &, bool)
{
    e.selectedFaceCount = 0;
}

static void do_wireframe_toggle(Editor &e, const Args &, bool)
{
    e.wireframe = !e.wireframe;
}

static void do_cycle(Editor &e, const Args &a, bool)
{
    int step = a.getInt("step");
    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);

    if (isFace)
    {
        int total = e.buffer.drawCmds[e.selectedRef].indicesCount / 3;
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
            if (j >= e.buffer.slotCount)
            {
                j = 1;
            }
            if (j == e.selectedRef)
            {
                return;
            }
            if (e.buffer.usedSlots[j])
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
                j = e.buffer.slotCount - 1;
            }
            if (j == e.selectedRef)
            {
                return;
            }
            if (e.buffer.usedSlots[j])
            {
                e.selectedRef = j;
                e.faceCursor = 0;
                e.selectedFaceCount = 0;
                return;
            }
        }
    }
}

static void do_reset(Editor &e, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    if (strcmp(a.get("property"), "rotation") == 0)
    {
        e.transforms[e.selectedRef].r = mat4::IDENTITY;
    }
    else
    {
        e.transforms[e.selectedRef].tx = 0;
        e.transforms[e.selectedRef].ty = 0;
        e.transforms[e.selectedRef].tz = 0;
    }
    buffer::recompose_model(e.buffer, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
}

static void do_undo(Editor &e, const Args &, bool snapshot)
{
    if (snapshot)
    {
        editor::pop_undo(e);
    }
}

static void do_quit(Editor &e, const Args &, bool)
{
    e.shouldQuit = true;
}

static void do_translate(Editor &e, const Args &, bool snapshot)
{
    const char *axis = e.args.get("axis");
    if (!axis)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    const char *lock = e.args.get("lock");
    if (lock && strcmp(lock, axis))
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    int step = e.args.getInt("step");
    float sens = e.keySensitivity;

    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);
    if (isFace && e.selectedFaceCount == 0)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }

    if (snapshot)
    {
        editor::push_undo(e);
    }

    if (isFace)
    {
        Mat4 delta;
        if (axis[0] == 'x')
        {
            delta = trs::translation(step * sens, 0, 0);
        }
        else if (axis[0] == 'y')
        {
            delta = trs::translation(0, step * sens, 0);
        }
        else
        {
            delta = trs::translation(0, 0, step * sens);
        }
        mesh::transform_faces(e.buffer, e.selectedRef, e.selectedFaces, e.selectedFaceCount, delta);
    }
    else
    {
        TRS &t = e.transforms[e.selectedRef];
        if (lock == nullptr)
        {
            if (axis[0] == 'x')
            {
                trs::translate(t, step * sens, 0, 0);
            }
            else if (axis[0] == 'y')
            {
                trs::translate(t, 0, step * sens, 0);
            }
            else
            {
                trs::translate(t, 0, 0, step * sens);
            }
        }
        else
        {
            if (axis[0] == 'x')
            {
                trs::translate_local(t, step * sens, 0, 0);
            }
            else if (axis[0] == 'y')
            {
                trs::translate_local(t, 0, step * sens, 0);
            }
            else
            {
                trs::translate_local(t, 0, 0, step * sens);
            }
        }
        buffer::recompose_model(e.buffer, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
    }

    e.args.clear("axis");
    e.args.clear("step");
}

static void do_rotate(Editor &e, const Args &, bool snapshot)
{
    const char *axis = e.args.get("axis");
    if (!axis)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    const char *lock = e.args.get("lock");
    if (lock && strcmp(lock, axis))
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    int step = e.args.getInt("step");
    float sens = e.keySensitivity;

    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);
    if (isFace && e.selectedFaceCount == 0)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }

    if (snapshot)
    {
        editor::push_undo(e);
    }

    if (isFace)
    {
        Mat4 delta;
        if (axis[0] == 'x')
        {
            delta = trs::rotation_x(step * sens);
        }
        else if (axis[0] == 'y')
        {
            delta = trs::rotation_y(step * sens);
        }
        else
        {
            delta = trs::rotation_z(step * sens);
        }
        mesh::transform_faces(e.buffer, e.selectedRef, e.selectedFaces, e.selectedFaceCount, delta);
    }
    else
    {
        TRS &t = e.transforms[e.selectedRef];
        if (lock == nullptr)
        {
            if (axis[0] == 'x')
            {
                t.r = trs::rotation_x(step * sens) * t.r;
            }
            else if (axis[0] == 'y')
            {
                t.r = trs::rotation_y(step * sens) * t.r;
            }
            else
            {
                t.r = trs::rotation_z(step * sens) * t.r;
            }
        }
        else
        {
            if (axis[0] == 'x')
            {
                t.r = t.r * trs::rotation_x(step * sens);
            }
            else if (axis[0] == 'y')
            {
                t.r = t.r * trs::rotation_y(step * sens);
            }
            else
            {
                t.r = t.r * trs::rotation_z(step * sens);
            }
        }
        buffer::recompose_model(e.buffer, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
    }

    e.args.clear("axis");
    e.args.clear("step");
}

static void do_scale(Editor &e, const Args &, bool snapshot)
{
    const char *axis = e.args.get("axis");
    if (!axis)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    const char *lock = e.args.get("lock");
    if (lock && strcmp(lock, axis))
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    int step = e.args.getInt("step");
    float sens = e.keySensitivity;

    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);
    if (isFace && e.selectedFaceCount == 0)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }

    if (snapshot)
    {
        editor::push_undo(e);
    }

    if (isFace)
    {
        Mat4 delta;
        if (axis[0] == 'x')
        {
            delta = trs::scaling(1.0f + step * sens, 1, 1);
        }
        else if (axis[0] == 'y')
        {
            delta = trs::scaling(1, 1.0f + step * sens, 1);
        }
        else
        {
            delta = trs::scaling(1, 1, 1.0f + step * sens);
        }
        mesh::transform_faces(e.buffer, e.selectedRef, e.selectedFaces, e.selectedFaceCount, delta);
    }
    else
    {
        TRS &t = e.transforms[e.selectedRef];
        if (axis[0] == 'x')
        {
            trs::scale(t, 1.0f + step * sens, 1, 1);
        }
        else if (axis[0] == 'y')
        {
            trs::scale(t, 1, 1.0f + step * sens, 1);
        }
        else
        {
            trs::scale(t, 1, 1, 1.0f + step * sens);
        }
        buffer::recompose_model(e.buffer, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
    }

    e.args.clear("axis");
    e.args.clear("step");
}

static void do_shear(Editor &e, const Args &, bool snapshot)
{
    const char *axis = e.args.get("axis");
    if (!axis)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    const char *lock = e.args.get("lock");
    if (lock && strcmp(lock, axis))
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }
    int step = e.args.getInt("step");
    float sens = e.keySensitivity;

    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);
    if (isFace && e.selectedFaceCount == 0)
    {
        e.args.clear("axis");
        e.args.clear("step");
        return;
    }

    if (snapshot)
    {
        editor::push_undo(e);
    }

    if (isFace)
    {
        Mat4 delta;
        if (axis[0] == 'x')
        {
            delta = trs::shearing(step * sens, 0, 0, 0, 0, 0);
        }
        else if (axis[0] == 'y')
        {
            delta = trs::shearing(0, 0, 0, step * sens, 0, 0);
        }
        else
        {
            delta = trs::shearing(0, 0, 0, 0, 0, step * sens);
        }
        mesh::transform_faces(e.buffer, e.selectedRef, e.selectedFaces, e.selectedFaceCount, delta);
    }
    else
    {
        TRS &t = e.transforms[e.selectedRef];
        if (axis[0] == 'x')
        {
            trs::shear(t, step * sens, 0, 0, 0, 0, 0);
        }
        else if (axis[0] == 'y')
        {
            trs::shear(t, 0, 0, 0, step * sens, 0, 0);
        }
        else
        {
            trs::shear(t, 0, 0, 0, 0, 0, step * sens);
        }
        buffer::recompose_model(e.buffer, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
    }

    e.args.clear("axis");
    e.args.clear("step");
}

// --- One-shot object ops ---------------------------------------------------

static void do_extrude(Editor &e, const Args &, bool snapshot)
{
    const char *target = e.args.get("target");
    bool isFace = (target != nullptr) && (strcmp(target, "face") == 0);
    if (isFace && e.selectedFaceCount == 0)
    {
        return;
    }
    if (snapshot)
    {
        editor::push_undo(e);
    }
    mesh::extrude_faces(e.buffer, e.selectedRef, e.selectedFaces, e.selectedFaceCount);
    e.selectedFaceCount = 0;
    e.cmd = &cmd::none;
}

static void do_del(Editor &e, const Args &, bool snapshot)
{
    if (!e.buffer.usedSlots[e.selectedRef])
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
        if (j >= e.buffer.slotCount)
        {
            j = 1;
        }
        if (j == oldRef)
        {
            break;
        }
        if (e.buffer.usedSlots[j])
        {
            e.selectedRef = j;
            break;
        }
    }

    buffer::remove(e.buffer, oldRef);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = &cmd::none;
    e.buffer.meshDirty = true;
}

static void do_merge(Editor &e, const Args &, bool snapshot)
{
    int usedCount = 0;
    for (int j = 1; j < e.buffer.slotCount; j++)
    {
        if (e.buffer.usedSlots[j])
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
        if (j >= e.buffer.slotCount)
        {
            j = 1;
        }
        if (e.buffer.usedSlots[j])
        {
            nextUsedSlot = j;
            break;
        }
    }

    e.selectedRef = buffer::merge(e.buffer, e.selectedRef, nextUsedSlot);
    e.faceCursor = 0;
    e.selectedFaceCount = 0;
    e.cmd = &cmd::none;
}

static void do_add_shape(Editor &e, const Args &a, bool snapshot)
{
    if (snapshot)
    {
        editor::push_undo(e);
    }
    const char *name = a.get("name");
    int param = a.getInt("param");

    if (strcmp(name, "cube") != 0 && strcmp(name, "pyramid") != 0 && strcmp(name, "cylinder") != 0)
    {
        printf("E: unknown shape: %s (valid: cube, pyramid, cylinder)\n", name);
        return;
    }

    Mesh mesh = (strcmp(name, "cube") == 0)      ? mesh::make_polyhedron(6)
                : (strcmp(name, "pyramid") == 0) ? mesh::make_pyramid(param > 0 ? param : 4)
                                                 : mesh::make_cylinder(param > 0 ? param : 8);

    if (!buffer::can_add(e.buffer, Mesh{.vtxCount = mesh.vtxCount, .idxCount = mesh.idxCount, .name = name}))
    {
        return;
    }

    TRS t;
    t.sx = t.sy = t.sz = e.defaultMeshScale;
    Ref newRef = buffer::add(e.buffer, Mesh{mesh.vertices, mesh.vtxCount, mesh.indices, mesh.idxCount, mesh.faceColors,
                                            mesh.faceCount, mesh.faceVtxCount, trs::compose(t), name});
    e.transforms[newRef] = t;

    e.selectedRef = newRef;
    e.faceCursor = 0;
    e.selectedFaceCount = 0;

    printf("added %s (object %d)\n", name, newRef);
}

static void do_save(Editor &e, const Args &a, bool)
{
    const char *file = a.get("file");
    if (file[0] != '\0')
    {
        strncpy(e.currentFile, file, sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
    }
    else if (e.currentFile[0] == '\0')
    {
        strncpy(e.currentFile, "handmade.obj", sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
    }
    wfront::save(e.buffer, e.selectedRef, e.currentFile);
}

static void do_load(Editor &e, const Args &a, bool)
{
    const char *file = a.get("file");
    bool clear = (strcmp(a.get("clear"), "true") == 0);
    if (wfront::load(e.buffer, e, file, clear))
    {
        strncpy(e.currentFile, file, sizeof(e.currentFile) - 1);
        e.currentFile[sizeof(e.currentFile) - 1] = '\0';
    }
}

static void do_help_cmd(Editor &, const Args &a, bool);

static void do_list(Editor &, const Args &, bool);

} // namespace cmd

// --- Registry -------------------------------------------------------------

const Command cmd::none = {"none", "no active command", nullptr, cmd::do_none};
const Command cmd::set_cmd = {"set_cmd", "switch current command", cmd::params_set_cmd, cmd::do_set_cmd};
const Command cmd::set_args = {"set_args", "set axis and step for the active transform", cmd::params_set_args,
                               cmd::do_set_args};
const Command cmd::set_target = {"set_target", "switch between object and face target", cmd::params_set_target,
                                 cmd::do_set_target};
const Command cmd::set_lock = {"set_lock", "toggle axis lock for transform commands", cmd::params_set_lock,
                               cmd::do_set_lock};
const Command cmd::toggle_selection = {"toggle_selection", "toggle selection of the active face (face target only)",
                                       nullptr, cmd::do_toggle_selection};
const Command cmd::clear_selection = {"clear_selection", "clear all selected faces", nullptr, cmd::do_clear_selection};
const Command cmd::wireframe_toggle = {"wireframe_toggle", "toggle wireframe rendering", nullptr,
                                       cmd::do_wireframe_toggle};
const Command cmd::cycle = {"cycle", "cycle next/prev through objects or faces", cmd::params_cycle, cmd::do_cycle};
const Command cmd::reset = {"reset", "reset rotation or translation of selected object", cmd::params_reset,
                            cmd::do_reset};
const Command cmd::undo = {"undo", "undo the last mutating command", nullptr, cmd::do_undo};
const Command cmd::quit = {"quit", "exit the application", nullptr, cmd::do_quit};
const Command cmd::translate = {"translate", "translate by signed step on axis (consumes axis+step)", nullptr,
                                cmd::do_translate};
const Command cmd::rotate = {"rotate", "rotate by signed step around axis (consumes axis+step)", nullptr,
                             cmd::do_rotate};
const Command cmd::scale = {"scale", "scale by signed step on axis (consumes axis+step)", nullptr, cmd::do_scale};
const Command cmd::shear = {"shear", "shear by signed step on axis (consumes axis+step)", nullptr, cmd::do_shear};
const Command cmd::extrude = {"extrude", "extrude selected faces (one-shot, resets to none)", nullptr, cmd::do_extrude};
const Command cmd::del = {"delete", "delete the selected object (one-shot)", nullptr, cmd::do_del};
const Command cmd::merge = {"merge", "merge selected object with the next one (one-shot)", nullptr, cmd::do_merge};
const Command cmd::add_shape = {"add_shape", "add a primitive shape to the scene", cmd::params_add_shape,
                                cmd::do_add_shape};
const Command cmd::save = {"save", "save the current scene to a file", cmd::params_save, cmd::do_save};
const Command cmd::load = {"load", "load a scene from a file", cmd::params_load, cmd::do_load};
const Command cmd::help = {"help", "show help for a command (or list all)", cmd::params_help, cmd::do_help_cmd};
const Command cmd::list = {"list", "list all registered commands", nullptr, cmd::do_list};

static const Command cmd_registry[] = {
    cmd::none,
    cmd::set_cmd,
    cmd::set_args,
    cmd::set_target,
    cmd::set_lock,
    cmd::toggle_selection,
    cmd::clear_selection,
    cmd::wireframe_toggle,
    cmd::cycle,
    cmd::reset,
    cmd::undo,
    cmd::quit,
    cmd::translate,
    cmd::rotate,
    cmd::scale,
    cmd::shear,
    cmd::extrude,
    cmd::del,
    cmd::merge,
    cmd::add_shape,
    cmd::save,
    cmd::load,
    cmd::help,
    cmd::list,
};

static const int cmd_registry_count = sizeof(cmd_registry) / sizeof(cmd_registry[0]);

namespace cmd
{

// --- Help / list bodies ----------------------------------------------------

static void do_help_cmd(Editor &, const Args &a, bool)
{
    const char *name = a.get("name");
    if (name == nullptr || name[0] == '\0')
    {
        for (int i = 0; i < cmd_registry_count; i++)
        {
            const Command &c = cmd_registry[i];
            printf("%s - %s\n", c.name, c.help);
        }
        return;
    }
    const Command *c = find(name);
    if (c == nullptr)
    {
        printf("E: unknown command '%s'\n", name);
        return;
    }
    printf("%s - %s\n", c->name, c->help);
    if (c->params != nullptr)
    {
        for (int i = 0; c->params[i].name != nullptr; i++)
        {
            const ParamSpec &p = c->params[i];
            printf("  %s %s", p.name, (p.type == ParamType::INT) ? "int" : "string");
            if (p.choices != nullptr)
            {
                printf(" [");
                for (int k = 0; p.choices[k] != nullptr; k++)
                {
                    if (k > 0)
                    {
                        printf("|");
                    }
                    printf("%s", p.choices[k]);
                }
                printf("]");
            }
            if (p.required)
            {
                printf(" (required)");
            }
            else if (p.defaultVal != nullptr)
            {
                printf(" (default %s)", p.defaultVal);
            }
            printf(" - %s\n", p.help);
        }
    }
}

static void do_list(Editor &, const Args &, bool)
{
    for (int i = 0; i < cmd_registry_count; i++)
    {
        const Command &c = cmd_registry[i];
        printf("%s - %s\n", c.name, c.help);
    }
}

// --- Registry lookup -------------------------------------------------------

const Command *find(const char *name)
{
    for (int i = 0; i < cmd_registry_count; i++)
    {
        if (strcmp(cmd_registry[i].name, name) == 0)
        {
            return &cmd_registry[i];
        }
    }
    return nullptr;
}

// --- Parser ----------------------------------------------------------------
//
// Splits `argsStr` on whitespace, classifies each token as NAMED (contains '=')
// or POSITIONAL, validates against `params`, and fills `out`.
//
// Token storage: a file-static buffer so values can outlive the parse_args
// frame (callers like cmd::run consume args immediately and the app is
// single-threaded). Documented non-re-entrancy.

static char tokenbuf[256];

bool parse_args(const char *argsStr, const ParamSpec *params, Args &out)
{
    out.count = 0;

    int len = strlen(argsStr);
    if (len >= (int)sizeof(tokenbuf))
    {
        printf("E: argument string too long (max %d)\n", (int)sizeof(tokenbuf) - 1);
        return false;
    }
    memcpy(tokenbuf, argsStr, len);
    tokenbuf[len] = '\0';

    // Phase 1: tokenize (split on whitespace, null-terminate in place).
    char *tokens[64];
    int ntok = 0;
    bool in_token = false;
    int start = 0;
    for (int i = 0; i <= len; i++)
    {
        bool is_ws = (tokenbuf[i] == ' ' || tokenbuf[i] == '\t' || tokenbuf[i] == '\0');
        if (is_ws)
        {
            if (in_token)
            {
                tokenbuf[i] = '\0';
                tokens[ntok++] = tokenbuf + start;
                in_token = false;
            }
        }
        else
        {
            if (!in_token)
            {
                start = i;
                in_token = true;
            }
        }
    }

    auto find_param = [&](const char *name) -> int {
        for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
        {
            if (strcmp(params[i].name, name) == 0)
            {
                return i;
            }
        }
        return -1;
    };

    auto already_set = [&](const char *name) -> bool {
        for (int i = 0; i < out.count; i++)
        {
            if (strcmp(out.names[i], name) == 0)
            {
                return true;
            }
        }
        return false;
    };

    // Phase 2: assign each token.
    for (int t = 0; t < ntok; t++)
    {
        char *eq = strchr(tokens[t], '=');
        if (eq != nullptr)
        {
            // NAMED token.
            *eq = '\0';
            const char *name = tokens[t];
            const char *value = eq + 1;

            int idx = find_param(name);
            if (idx < 0)
            {
                printf("E: unknown parameter '%s'\n", name);
                return false;
            }
            const ParamSpec &p = params[idx];

            if (already_set(p.name))
            {
                printf("E: parameter '%s' specified more than once\n", p.name);
                return false;
            }

            if (p.type == ParamType::INT)
            {
                char *end = nullptr;
                long v = strtol(value, &end, 10);
                if (end == value || (*end != '\0'))
                {
                    printf("E: '%s' expects an integer\n", p.name);
                    return false;
                }
                (void)v;
            }
            else if (p.type == ParamType::STRING && p.choices != nullptr)
            {
                bool found = false;
                for (int i = 0; p.choices[i] != nullptr; i++)
                {
                    if (strcmp(p.choices[i], value) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    printf("E: bad value for '%s' (expected one of ", p.name);
                    for (int i = 0; p.choices[i] != nullptr; i++)
                    {
                        if (i > 0)
                        {
                            printf("|");
                        }
                        printf("%s", p.choices[i]);
                    }
                    printf(")\n");
                    return false;
                }
            }

            out.names[out.count] = p.name;
            out.values[out.count] = value;
            out.count++;
        }
        else
        {
            // POSITIONAL token.
            int idx = -1;
            for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
            {
                if (!already_set(params[i].name))
                {
                    idx = i;
                    break;
                }
            }
            if (idx < 0)
            {
                printf("E: too many positional arguments\n");
                return false;
            }
            const ParamSpec &p = params[idx];
            const char *value = tokens[t];

            if (p.type == ParamType::INT)
            {
                char *end = nullptr;
                long v = strtol(value, &end, 10);
                if (end == value || (*end != '\0'))
                {
                    printf("E: '%s' expects an integer\n", p.name);
                    return false;
                }
                (void)v;
            }
            else if (p.type == ParamType::STRING && p.choices != nullptr)
            {
                bool found = false;
                for (int i = 0; p.choices[i] != nullptr; i++)
                {
                    if (strcmp(p.choices[i], value) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    printf("E: bad value for '%s' (expected one of ", p.name);
                    for (int i = 0; p.choices[i] != nullptr; i++)
                    {
                        if (i > 0)
                        {
                            printf("|");
                        }
                        printf("%s", p.choices[i]);
                    }
                    printf(")\n");
                    return false;
                }
            }

            out.names[out.count] = p.name;
            out.values[out.count] = value;
            out.count++;
        }
    }

    // Phase 3: required-vs-present check, fill optional defaults.
    for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
    {
        if (already_set(params[i].name))
        {
            continue;
        }
        if (params[i].required)
        {
            printf("E: missing required parameter '%s'\n", params[i].name);
            return false;
        }
        out.names[out.count] = params[i].name;
        out.values[out.count] = (params[i].defaultVal != nullptr) ? params[i].defaultVal : "";
        out.count++;
    }

    return true;
}

// --- Dispatcher ------------------------------------------------------------

void run(Editor &e, const Command *c, const char *argsStr, bool snapshot)
{
    Args a;
    if (!parse_args(argsStr, c->params, a))
    {
        return;
    }
    c->action(e, a, snapshot);
    e.cmd->action(e, e.args, snapshot);
}

} // namespace cmd
