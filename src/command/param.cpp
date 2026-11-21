#include "param.h"

#include <stdlib.h>
#include <string.h>

namespace param
{

void add(Args &a, const char *name, const char *value)
{
    if (a.count >= MAX_PARAMS)
    {
        return;
    }

    a.names[a.count] = name;
    strncpy(a.values[a.count], value, Args::MAX_VALUE - 1);
    a.values[a.count][Args::MAX_VALUE - 1] = '\0';
    a.count++;
}

const char *get(const Args &a, const char *name)
{
    for (int i = 0; i < a.count; i++)
    {
        if (strcmp(a.names[i], name) == 0)
        {
            return a.values[i];
        }
    }
    return nullptr;
}

int get_int(const Args &a, const char *name)
{
    const char *v = get(a, name);
    return v ? atoi(v) : 0;
}

float get_float(const Args &a, const char *name)
{
    const char *v = get(a, name);
    return v ? strtof(v, nullptr) : 0.0f;
}

const char *type_name(ParamType type)
{
    switch (type)
    {
    case ParamType::INT:
        return "int";
    case ParamType::FLOAT:
        return "float";
    case ParamType::STRING:
        return "string";
    }
    return "string";
}

} // namespace param
