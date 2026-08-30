#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Editor;

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
    const char *names[MAX_PARAMS];
    const char *values[MAX_PARAMS];
    char valueStorage[MAX_PARAMS][64];
    int count = 0;

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

    void set(const char *name, const char *value)
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(names[i], name) == 0)
            {
                strncpy(valueStorage[i], value, sizeof(valueStorage[i]) - 1);
                valueStorage[i][sizeof(valueStorage[i]) - 1] = '\0';
                values[i] = valueStorage[i];
                return;
            }
        }
        if (count >= MAX_PARAMS)
        {
            printf("E: args: MAX_PARAMS (%d) exceeded\n", MAX_PARAMS);
            fflush(stdout);
            abort();
        }
        names[count] = name;
        strncpy(valueStorage[count], value, sizeof(valueStorage[count]) - 1);
        valueStorage[count][sizeof(valueStorage[count]) - 1] = '\0';
        values[count] = valueStorage[count];
        count++;
    }

    void clear(const char *name)
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(names[i], name) == 0)
            {
                for (int j = i; j < count - 1; j++)
                {
                    names[j] = names[j + 1];
                    values[j] = values[j + 1];
                    for (size_t k = 0; k < sizeof(valueStorage[j]); k++)
                    {
                        valueStorage[j][k] = valueStorage[j + 1][k];
                    }
                }
                count--;
                return;
            }
        }
    }
};

using Action = void (*)(Editor &, const Args &, bool);

struct Command
{
    const char *name;
    const char *help;
    const ParamSpec *params;
    Action action;
};

namespace cmd
{
const Command *find(const char *name);
bool parse_args(const char *argsStr, const ParamSpec *params, Args &out);
void run(Editor &, const Command *, const char *argsStr, bool snapshot);

// `del` stands in for the `delete` command: `delete` is a C++ keyword
// and cannot be used as an identifier. The Command.name string is still
// "delete" (set in todo 9's command.cpp).
extern const Command none;
extern const Command set_cmd;
extern const Command set_args;
extern const Command set_target;
extern const Command set_lock;
extern const Command toggle_selection;
extern const Command clear_selection;
extern const Command wireframe_toggle;
extern const Command cycle;
extern const Command reset;
extern const Command undo;
extern const Command quit;
extern const Command translate;
extern const Command rotate;
extern const Command scale;
extern const Command shear;
extern const Command extrude;
extern const Command del;
extern const Command merge;
extern const Command add_shape;
extern const Command save;
extern const Command load;
extern const Command help;
extern const Command list;
} // namespace cmd

#endif
