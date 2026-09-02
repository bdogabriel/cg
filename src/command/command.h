#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>
#include <string.h>

// TODO: add duplicate command

struct Editor;
struct Session;

enum class ParamType
{
    INT,
    STRING
};

struct ParamSpec
{
    const char *name;
    ParamType type;
    const char *const *choices;
    const char *help;
    bool required;
    const char *defaultVal;
};

constexpr int MAX_PARAMS = 8;

struct Args
{
    static constexpr int MAX_VALUE = 256;
    const char *names[MAX_PARAMS];
    char values[MAX_PARAMS][MAX_VALUE];
    int count = 0;

    void set(int i, const char *name, const char *value)
    {
        names[i] = name;
        strncpy(values[i], value, MAX_VALUE - 1);
        values[i][MAX_VALUE - 1] = '\0';
    }

    const char *get(const char *name) const
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(names[i], name) == 0)
            {
                return values[i];
            }
        }
        return nullptr;
    }

    int getInt(const char *name) const
    {
        const char *v = get(name);
        return v ? atoi(v) : 0;
    }
};

using Action = void (*)(Editor &, Session &, const Args &, bool);

struct Command
{
    const char *name;
    const char *help;
    const ParamSpec *params;
    Action action;
};

struct CommandCall
{
    const Command *cmd;
    const char *args;
    bool snapshot;
};

enum class CommandId : int
{
#define CMD(id, name, help, params, action) id,
#include "command.def"
#undef CMD
    COUNT
};

namespace cmd
{
const Command *find(const char *name);
const Command *get(CommandId id);
bool parse_args(const char *argsStr, const ParamSpec *params, Args &out);
void run(Editor &, Session &, const Command *, const char *argsStr, bool snapshot);
} // namespace cmd

#endif
