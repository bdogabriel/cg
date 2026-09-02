#include "binding.h"
#include <stdio.h>

namespace binding
{

Binding bindings[MAX_BINDINGS];
int bindingCount = 0;

void bind(Key key, uint8_t mods, bool oneShot, const char *cmdName, const char *args)
{
    if (bindingCount >= MAX_BINDINGS)
    {
        printf("E: binding: MAX_BINDINGS (%d) exceeded\n", MAX_BINDINGS);
        fflush(stdout);
        exit(1);
    }

    const Command *c = cmd::find(cmdName);
    if (c == nullptr)
    {
        printf("E: binding: unknown command '%s'\n", cmdName);
        fflush(stdout);
        exit(1);
    }

    for (int i = 0; i < bindingCount; i++)
    {
        if (bindings[i].key == key && bindings[i].mods == mods)
        {
            printf("E: binding collision at key %d mods %u\n", (int)key, (unsigned)mods);
            fflush(stdout);
            exit(1);
        }
    }

    Binding &b = bindings[bindingCount];
    b.key = key;
    b.mods = mods;
    b.oneShot = oneShot;
    b.cmd = c;
    b.args = args;
    bindingCount++;
}

const Binding *find(Key key, uint8_t mods)
{
    for (int i = 0; i < bindingCount; i++)
    {
        if (bindings[i].key == key && bindings[i].mods == mods)
        {
            return &bindings[i];
        }
    }
    return nullptr;
}

int resolve(const Keyboard &kb, CommandCall *out, int maxOut)
{
    int count = 0;
    for (int key = 0; key < (int)Key::COUNT && count < maxOut; key++)
    {
        KeyState ks = kb.keyStates[key];
        if (ks != KeyState::JUST_PRESSED && ks != KeyState::DOWN)
        {
            continue;
        }
        const Binding *b = find((Key)key, kb.mods);
        if (!b)
        {
            continue;
        }
        bool triggered = ks == KeyState::JUST_PRESSED || (!b->oneShot && ks == KeyState::DOWN);
        if (!triggered)
        {
            continue;
        }
        out[count++] = {b->cmd, b->args, ks == KeyState::JUST_PRESSED};
    }
    return count;
}

} // namespace binding
