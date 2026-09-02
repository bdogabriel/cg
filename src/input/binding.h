#ifndef BINDING_H
#define BINDING_H

#include "command.h"
#include "keyboard.h"

// FIX: visual mode

struct Binding
{
    Key key;
    uint8_t mods;
    bool oneShot;
    const Command *cmd;
    const char *args;
};

namespace binding
{

constexpr int MAX_BINDINGS = 64;

const Binding *find(Key key, uint8_t mods);
void bind(Key key, uint8_t mods, bool oneShot, const char *cmdName, const char *args);
int resolve(const Keyboard &kb, CommandCall *out, int maxOut);

} // namespace binding

#endif
