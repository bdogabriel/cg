#include "command_actions.h"
#include "editor.h"
#include "prompt.h"
#include "session.h"
#include <cstdio>

namespace cmd
{

static const char *const target_choices[] = {"object", "face", nullptr};

const ParamSpec params_set_cmd[] = {
    {"op", ParamType::STRING, cmd_choices, "command to switch into", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_set_target[] = {
    {"t", ParamType::STRING, target_choices, "object or face target", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

void do_none(Editor &, Session &, const Args &, bool)
{
}

void do_prompt(Editor &e, Session &s, const Args &, bool)
{
    (void)e;
    prompt::reset(s.prompt);
    s.cmd = get(CommandId::execute);
}

void do_execute(Editor &e, Session &s, const Args &, bool snapshot)
{
    if (s.prompt.buffer[0] == '\0')
    {
        return;
    }
    s.cmd = get(CommandId::none);

    char buf[512];
    strncpy(buf, s.prompt.buffer, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *sp = strchr(buf, ' ');
    const char *name = buf;
    const char *args = "";
    if (sp)
    {
        *sp = '\0';
        args = sp + 1;
    }
    if (name[0] == '\0')
    {
        return;
    }
    const Command *c = find(name);
    if (!c)
    {
        printf("E: unknown command: %s\n", name);
        return;
    }
    run(e, s, c, args, snapshot);
}

void do_set_cmd(Editor &e, Session &s, const Args &a, bool snapshot)
{
    const char *op = a.get("op");
    const Command *c = find(op);
    if (!c)
    {
        return;
    }
    s.cmd = c;
    if (c == get(CommandId::none))
    {
        e.selectedFaceCount = 0;
    }
    if (c == get(CommandId::extrude))
    {
        e.target = Target::Face;
    }
    s.cmd->action(e, s, s.args, snapshot);
}

void do_set_args(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)a;
    s.cmd->action(e, s, s.args, snapshot);
}

void do_set_target(Editor &e, Session &s, const Args &a, bool)
{
    (void)s;
    const char *t = a.get("t");
    e.target = (strcmp(t, "face") == 0) ? Target::Face : Target::Object;
    e.selectedFaceCount = 0;
}

void do_toggle_lock(Editor &e, Session &, const Args &, bool)
{
    e.locked = !e.locked;
}

void do_set_bg_color(Editor &e, Session &, const Args &a, bool)
{
    Color c;
    if (!color::from_hex(a.get("color"), c))
    {
        printf("E: invalid color '%s' (expected #RRGGBB or #RRGGBBAA)\n", a.get("color"));
        return;
    }
    e.bgColor = c;
}

void do_toggle_selection(Editor &e, Session &s, const Args &, bool)
{
    (void)s;
    if (e.target == Target::Object)
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

void do_clear_selection(Editor &e, Session &, const Args &, bool)
{
    e.selectedFaceCount = 0;
}

void do_wireframe_toggle(Editor &e, Session &, const Args &, bool)
{
    e.wireframe = !e.wireframe;
}

void do_quit(Editor &, Session &s, const Args &, bool)
{
    s.shouldQuit = true;
}

} // namespace cmd
