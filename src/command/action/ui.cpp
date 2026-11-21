#include "action.h"
#include "command.h"
#include "editor.h"
#include "prompt.h"
#include <cstdio>

namespace cmd
{

namespace
{

const char *const target_choices[] = {"object", "face", nullptr};

} // namespace

const ParamSpec params_set_cmd[] = {
    {"op", ParamType::STRING, cmd_choices, "command to switch into", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

const ParamSpec params_set_target[] = {
    {"t", ParamType::STRING, target_choices, "object or face target", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

void do_none(Editor &, bool)
{
}

void do_prompt(Editor &e, bool)
{
    prompt::reset(e.prompt);
    e.cmd = CommandId::execute;
}

void do_execute(Editor &e, bool justPressed)
{
    if (e.prompt.buffer[0] == '\0')
    {
        return;
    }
    e.cmd = CommandId::camera;

    char buf[512];
    strncpy(buf, e.prompt.buffer, sizeof(buf) - 1);
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
    CommandId id;
    if (!find_id(name, id))
    {
        printf("E: unknown command: %s\n", name);
        return;
    }
    run(e, id, args, justPressed);
}

void do_set_cmd(Editor &e, bool justPressed)
{
    const char *op = param::get(e.args, "op");
    const Command *c = find(op);
    if (!c)
    {
        return;
    }
    e.cmd = c->id;
    if (e.cmd == CommandId::none)
    {
        e.selectedFaceCount = 0;
    }
    if (e.cmd == CommandId::extrude)
    {
        e.target = Target::FACE;
    }
    c->action(e, justPressed);
}

void do_set_args(Editor &e, bool justPressed)
{
    get(e.cmd)->action(e, justPressed);
}

void do_set_target(Editor &e, bool)
{
    const char *t = param::get(e.args, "t");
    e.target = (strcmp(t, "face") == 0) ? Target::FACE : Target::OBJECT;
    e.selectedFaceCount = 0;
}

void do_toggle_lock(Editor &e, bool)
{
    e.locked = !e.locked;
}

void do_set_bg_color(Editor &e, bool)
{
    Color c;
    if (!color::from_hex(param::get(e.args, "color"), c))
    {
        printf("E: invalid color '%s' (expected #RRGGBB or #RRGGBBAA)\n", param::get(e.args, "color"));
        return;
    }
    e.bgColor = c;
}

void do_toggle_selection(Editor &e, bool)
{
    if (e.target == Target::OBJECT)
    {
        return;
    }
    for (int i = 0; i < e.selectedFaceCount; i++)
    {
        if (BUF(int, e.selectedFaces)[i] == e.faceCursor)
        {
            BUF(int, e.selectedFaces)[i] = BUF(int, e.selectedFaces)[e.selectedFaceCount - 1];
            e.selectedFaceCount--;
            return;
        }
    }
    buffer::ensure(e.selectedFaces, sizeof(int), e.selectedFaceCount + 1);
    BUF(int, e.selectedFaces)[e.selectedFaceCount++] = e.faceCursor;
}

void do_clear_selection(Editor &e, bool)
{
    e.selectedFaceCount = 0;
}

void do_wireframe_toggle(Editor &e, bool)
{
    e.wireframe = !e.wireframe;
}

void do_quit(Editor &e, bool)
{
    window::request_close(e.window);
}

} // namespace cmd
