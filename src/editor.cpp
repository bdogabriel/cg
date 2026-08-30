#include "editor.h"
#include "binding.h"

namespace editor
{

void setup(Editor &e, Input &input)
{
    e.input = &input;
    e.args.set("target", "object");
}

void push_undo(Editor &e)
{
    int i = (e.undoHead + e.undoCount) % MAX_UNDO;
    e.undoBuffers[i] = e.buffer;
    memcpy(e.undoTransforms[i], e.transforms, sizeof(e.transforms));
    if (e.undoCount == MAX_UNDO)
    {
        e.undoHead = (e.undoHead + 1) % MAX_UNDO;
    }
    else
    {
        ++e.undoCount;
    }
}

void pop_undo(Editor &e)
{
    if (e.undoCount == 0)
    {
        return;
    }
    --e.undoCount;
    int i = (e.undoHead + e.undoCount) % MAX_UNDO;
    e.buffer = e.undoBuffers[i];
    memcpy(e.transforms, e.undoTransforms[i], sizeof(e.transforms));
    e.buffer.meshDirty = true;
    e.buffer.modelsDirty = true;
}

void process_input(Editor &e)
{
    Input &in = *e.input;

    if (in.keyStates[(int)Key::SEMICOLON] == KeyState::JUST_PRESSED && (in.mods & mods::SHIFT))
    {
        printf(":");
        fflush(stdout);
        char line[256];
        if (!fgets(line, sizeof(line), stdin))
        {
            return;
        }
        char *nl = strchr(line, '\n');
        if (nl)
        {
            *nl = '\0';
        }
        char *sp = strchr(line, ' ');
        const char *name = line;
        const char *args = "";
        if (sp)
        {
            *sp = '\0';
            args = sp + 1;
        }
        const Command *c = cmd::find(name);
        if (!c)
        {
            printf("E: unknown command: %s\n", name);
        }
        else
        {
            cmd::run(e, c, args, true);
        }
        return;
    }

    for (int key = 0; key < (int)Key::COUNT; key++)
    {
        KeyState ks = in.keyStates[key];
        if (ks != KeyState::JUST_PRESSED && ks != KeyState::DOWN)
        {
            continue;
        }
        const Binding *b = binding::find((Key)key, in.mods);
        if (!b)
        {
            continue;
        }
        bool triggered = ks == KeyState::JUST_PRESSED || (!b->oneShot && ks == KeyState::DOWN);
        if (!triggered)
        {
            continue;
        }
        cmd::run(e, b->cmd, b->args, (ks == KeyState::JUST_PRESSED));
    }
}

} // namespace editor
