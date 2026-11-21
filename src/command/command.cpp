#include "command.h"
#include "editor.h"
#include <cstdio>
#include <string.h>

namespace
{

constexpr const char *const AXIS_CHOICES[] = {"x", "y", "z", nullptr};
constexpr const char *const TARGET_CHOICES[] = {"object", "face", nullptr};
constexpr const char *const RESET_CHOICES[] = {"rotation", "translation", "scale", nullptr};
constexpr const char *const CLEAR_CHOICES[] = {"true", "false", nullptr};

constexpr Param AXIS{"axis", ParamType::STRING, AXIS_CHOICES, "axis to act on", true, nullptr};

constexpr Param STEP{
    "step", ParamType::INT, nullptr, "signed step size (negative = reverse)", true, nullptr};

constexpr Param DEGREES{"degrees", ParamType::FLOAT, nullptr, "signed angle in degrees", true, nullptr};

constexpr Param OP{
    "op", ParamType::STRING, nullptr, "command to switch into (see help commands)", true, nullptr};

constexpr Param TARGET{
    "target", ParamType::STRING, TARGET_CHOICES, "object or face target", true, nullptr};

constexpr Param PROPERTY{
    "property", ParamType::STRING, RESET_CHOICES, "which component to reset", true, nullptr};

constexpr Param COLOR_OPTIONAL{
    "color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", false, "ffffff"};

constexpr Param COLOR_REQUIRED{
    "color", ParamType::STRING, nullptr, "hex color (#RRGGBB or #RRGGBBAA)", true, nullptr};

constexpr Param SIDES{"sides", ParamType::INT, nullptr, "number of base sides", false, "4"};

constexpr Param SEGMENTS{
    "segments", ParamType::INT, nullptr, "number of side segments", false, "8"};

constexpr Param FACES{
    "faces", ParamType::INT, nullptr, "face count (4, 6, 8, 12, 20)", false, "6"};

constexpr Param RESOLUTION{
    "resolution", ParamType::INT, nullptr, "segments (slices and stacks)", false, "16"};

constexpr Param OUTPUT{
    "output", ParamType::STRING, nullptr, "output filename (default = current file)", false, ""};

constexpr Param NAME{"name", ParamType::STRING, nullptr, "scene name", true, nullptr};

constexpr Param INPUT_FILE{"file", ParamType::STRING, nullptr, "input filename", true, nullptr};

constexpr Param CLEAR{
    "clear", ParamType::STRING, CLEAR_CHOICES, "clear existing scene first", false, "false"};

constexpr Param ANGLE{
    "angle", ParamType::INT, nullptr, "angle tolerance in degrees (default 1)", false, "1"};

constexpr Param TOPIC{
    "topic", ParamType::STRING, nullptr, "'commands', 'bindings', or a command name", true, nullptr};

} // namespace

namespace cmd
{

void do_none(Editor &e, bool justPressed);
void do_prompt(Editor &e, bool justPressed);
void do_execute(Editor &e, bool justPressed);
void do_set_cmd(Editor &e, bool justPressed);
void do_set_args(Editor &e, bool justPressed);
void do_set_target(Editor &e, bool justPressed);
void do_toggle_lock(Editor &e, bool justPressed);
void do_set_color(Editor &e, bool justPressed);
void do_apply_color(Editor &e, bool justPressed);
void do_set_bg_color(Editor &e, bool justPressed);
void do_toggle_selection(Editor &e, bool justPressed);
void do_clear_selection(Editor &e, bool justPressed);
void do_wireframe_toggle(Editor &e, bool justPressed);
void do_cycle(Editor &e, bool justPressed);
void do_reset(Editor &e, bool justPressed);
void do_undo(Editor &e, bool justPressed);
void do_quit(Editor &e, bool justPressed);
void do_translate(Editor &e, bool justPressed);
void do_rotate(Editor &e, bool justPressed);
void do_rotate_degrees(Editor &e, bool justPressed);
void do_scale(Editor &e, bool justPressed);
void do_uniform_scale(Editor &e, bool justPressed);
void do_shear(Editor &e, bool justPressed);
void do_camera(Editor &e, bool justPressed);
void do_extrude(Editor &e, bool justPressed);
void do_delete(Editor &e, bool justPressed);
void do_merge(Editor &e, bool justPressed);
void do_merge_coplanar(Editor &e, bool justPressed);
void do_add_pyramid(Editor &e, bool justPressed);
void do_add_cylinder(Editor &e, bool justPressed);
void do_add_polyhedron(Editor &e, bool justPressed);
void do_add_sphere(Editor &e, bool justPressed);
void do_save_obj(Editor &e, bool justPressed);
void do_save_scene(Editor &e, bool justPressed);
void do_load_obj(Editor &e, bool justPressed);
void do_load_scene(Editor &e, bool justPressed);
void do_load_skybox(Editor &e, bool justPressed);
void do_help(Editor &e, bool justPressed);

namespace
{

const Command registry[] = {
    {"none", "no active command", {}, 0, do_none},
    {"prompt", "open the command prompt", {}, 0, do_prompt},
    {"execute", "execute the typed command", {}, 0, do_execute},
    {"set_cmd", "switch current command", {OP}, 1, do_set_cmd},
    {"set_args", "set axis and step for the active transform", {AXIS, STEP}, 2,
     do_set_args},
    {"set_target", "switch between object and face target", {TARGET}, 1, do_set_target},
    {"toggle_lock", "toggle local/world transform lock", {}, 0, do_toggle_lock},
    {"set_color", "set color of selected object/face", {COLOR_REQUIRED}, 1, do_set_color},
    {"set_bg_color", "set the background color", {COLOR_REQUIRED}, 1, do_set_bg_color},
    {"apply_color", "apply current color to selected object/face", {}, 0, do_apply_color},
    {"toggle_selection", "toggle selection of the active face", {}, 0, do_toggle_selection},
    {"clear_selection", "clear all selected faces", {}, 0, do_clear_selection},
    {"wireframe_toggle", "toggle wireframe rendering", {}, 0, do_wireframe_toggle},
    {"cycle", "cycle next/prev through objects or faces", {STEP}, 1, do_cycle},
    {"reset", "reset rotation, translation, or scale of selected object", {PROPERTY}, 1,
     do_reset},
    {"undo", "undo the last mutating command", {}, 0, do_undo},
    {"quit", "exit the application", {}, 0, do_quit},
    {"translate", "translate by signed step on axis", {AXIS, STEP}, 2, do_translate},
    {"rotate", "rotate by signed step around axis", {AXIS, STEP}, 2, do_rotate},
    {"rotate_degrees", "rotate by signed degrees around axis", {AXIS, DEGREES}, 2,
     do_rotate_degrees},
    {"scale", "scale by signed step on axis", {AXIS, STEP}, 2, do_scale},
    {"uniform_scale", "scale uniformly on all axes", {STEP}, 1, do_uniform_scale},
    {"shear", "shear by signed step on axis", {AXIS, STEP}, 2, do_shear},
    {"camera", "move/look the camera", {AXIS, STEP}, 2, do_camera},
    {"extrude", "extrude selected faces (one-shot)", {}, 0, do_extrude},
    {"delete", "delete the selected object (one-shot)", {}, 0, do_delete},
    {"merge", "merge selected object with the next one (one-shot)", {}, 0, do_merge},
    {"merge_coplanar", "merge coplanar faces of the selected object", {ANGLE}, 1,
     do_merge_coplanar},
    {"add_pyramid", "add a pyramid primitive", {SIDES, COLOR_OPTIONAL}, 2,
     do_add_pyramid},
    {"add_cylinder", "add a cylinder primitive", {SEGMENTS, COLOR_OPTIONAL}, 2,
     do_add_cylinder},
    {"add_polyhedron", "add a polyhedron primitive (cube = 6 faces)",
     {FACES, COLOR_OPTIONAL}, 2, do_add_polyhedron},
    {"add_sphere", "add a sphere primitive", {RESOLUTION, COLOR_OPTIONAL}, 2,
     do_add_sphere},
    {"save_obj", "save selected object to an .obj file", {OUTPUT}, 1, do_save_obj},
    {"save_scene", "save the scene to a file", {NAME}, 1, do_save_scene},
    {"load_scene", "load a scene from a file", {NAME, CLEAR}, 2, do_load_scene},
    {"load_skybox", "load an equirectangular skybox image", {INPUT_FILE}, 1, do_load_skybox},
    {"load_obj", "load a single .obj object", {INPUT_FILE, CLEAR}, 2, do_load_obj},
    {"help", "show help on a topic", {TOPIC}, 1, do_help},
};

} // namespace

const Command *find(const char *name)
{
    for (int i = 0; i < count(); i++)
    {
        if (strcmp(registry[i].name, name) == 0)
        {
            return &registry[i];
        }
    }
    return nullptr;
}

int count()
{
    return (int)(sizeof(registry) / sizeof(registry[0]));
}

const Command *at(int i)
{
    if (i < 0 || i >= count())
    {
        return nullptr;
    }
    return &registry[i];
}

namespace
{

char tokenbuf[512];

bool validate_param_value(const Param &p, const char *value)
{
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
    else if (p.type == ParamType::FLOAT)
    {
        char *end = nullptr;
        float v = strtof(value, &end);
        if (end == value || (*end != '\0'))
        {
            printf("E: '%s' expects a number\n", p.name);
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
    return true;
}

const Param *find_param(const Command &c, const char *name)
{
    for (int i = 0; i < c.paramCount; i++)
    {
        if (strcmp(c.params[i].name, name) == 0)
        {
            return &c.params[i];
        }
    }
    return nullptr;
}

bool is_already_set(const Args &args, const char *name)
{
    return param::get(args, name) != nullptr;
}

} // namespace

// TODO: extract/split
bool parse_args(const char *argsStr, const Command &c, Args &out)
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

    for (int t = 0; t < ntok; t++)
    {
        char *eq = strchr(tokens[t], '=');
        if (eq != nullptr)
        {
            *eq = '\0';
            const char *name = tokens[t];
            const char *value = eq + 1;

            const Param *p = find_param(c, name);
            if (!p)
            {
                printf("E: unknown parameter '%s'\n", name);
                return false;
            }

            if (is_already_set(out, p->name))
            {
                printf("E: parameter '%s' specified more than once\n", p->name);
                return false;
            }

            if (!validate_param_value(*p, value))
            {
                return false;
            }

            param::add(out, p->name, value);
        }
        else
        {
            int idx = -1;
            for (int i = 0; i < c.paramCount; i++)
            {
                if (!is_already_set(out, c.params[i].name))
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
            const Param &p = c.params[idx];
            const char *value = tokens[t];

            if (!validate_param_value(p, value))
            {
                return false;
            }

            param::add(out, p.name, value);
        }
    }

    for (int i = 0; i < c.paramCount; i++)
    {
        const Param &p = c.params[i];
        if (is_already_set(out, p.name))
        {
            continue;
        }
        if (p.required)
        {
            printf("E: missing required parameter '%s'\n", p.name);
            return false;
        }
        param::add(out, p.name, (p.defaultVal != nullptr) ? p.defaultVal : "");
    }

    return true;
}

void run(Editor &e, const Command *c, const char *argsStr, bool justPressed)
{
    if (!c)
    {
        return;
    }
    if (!parse_args(argsStr, *c, e.args))
    {
        print_command_help(*c);
        return;
    }
    c->action(e, justPressed);
}

bool is_exec(const Editor &e)
{
    return e.cmd && strcmp(e.cmd->name, "execute") == 0;
}

} // namespace cmd
