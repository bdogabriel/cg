#include "command.h"
#include "editor.h"
#include "prompt.h"
#include <cstdio>
#include <string.h>

namespace cmd
{

void do_none(Editor &, bool)
{
}

void do_prompt(Editor &e, bool)
{
    prompt::reset(e.prompt);
    e.cmd = find("execute");
}

void do_execute(Editor &e, bool justPressed)
{
    if (e.prompt.buffer[0] == '\0')
    {
        return;
    }
    e.cmd = find("camera");

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
    const Command *c = find(name);
    if (!c)
    {
        printf("E: unknown command: %s\n", name);
        return;
    }
    run(e, c, args, justPressed);
}

void do_set_cmd(Editor &e, bool justPressed)
{
    const char *op = param::get(e.args, "op");
    const Command *c = find(op);
    if (!c)
    {
        printf("E: unknown command: %s\n", op);
        return;
    }
    e.cmd = c;
    if (strcmp(c->name, "none") == 0)
    {
        e.selectedFaceCount = 0;
    }
    if (strcmp(c->name, "extrude") == 0)
    {
        e.target = Target::FACE;
    }
    c->action(e, justPressed);
}

void do_set_args(Editor &e, bool justPressed)
{
    if (e.cmd)
    {
        e.cmd->action(e, justPressed);
    }
}

void do_set_target(Editor &e, bool)
{
    const char *target = param::get(e.args, "target");
    e.target = (strcmp(target, "face") == 0) ? Target::FACE : Target::OBJECT;
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
