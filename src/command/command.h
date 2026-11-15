#ifndef COMMAND_H
#define COMMAND_H

#include "action.h"
#include "param.h"

// TODO: add duplicate command

struct Command
{
    CommandId id;
    const char *name;
    const char *help;
    const ParamSpec *params;
    Action action;
};

namespace cmd
{
const Command *find(const char *name);
bool find_id(const char *name, CommandId &out);
const Command *get(CommandId id);
int count();
const Command *at(int i);
void print_command_help(const Command &c);
bool parse_args(const char *argsStr, const ParamSpec *params, Args &out);
void run(Editor &e, CommandId id, const char *argsStr, bool justPressed);
} // namespace cmd

#endif
