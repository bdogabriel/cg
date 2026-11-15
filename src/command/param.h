#ifndef PARAM_H
#define PARAM_H

#include <stdlib.h>
#include <string.h>

constexpr int MAX_PARAMS = 8;

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

// TODO: move functions to namespace
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

enum class CommandId : int
{
#define CMD(id, name, help, params, action) id,
#include "command.def"
#undef CMD
    COUNT
};

#endif
