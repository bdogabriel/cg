#ifndef COMMAND_H
#define COMMAND_H

#include "param.h"

struct Editor;

using Action = void (*)(Editor &e, bool justPressed);

struct Command
{
    const char *name;
    const char *help;
    Param params[MAX_PARAMS];
    int paramCount;
    Action action;
};

namespace cmd
{
const Command *find(const char *name);
int count();
const Command *at(int i);
void print_command_help(const Command &c);
bool parse_args(const char *argsStr, const Command &c, Args &out);
void run(Editor &e, const Command *c, const char *argsStr, bool justPressed);
bool is_exec(const Editor &e);
} // namespace cmd

#endif
