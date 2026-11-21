#ifndef PARAM_H
#define PARAM_H

#include <stdlib.h>

constexpr int MAX_PARAMS = 8;

enum class ParamType
{
    INT,
    FLOAT,
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

struct Args
{
    static constexpr int MAX_VALUE = 256;
    const char *names[MAX_PARAMS];
    char values[MAX_PARAMS][MAX_VALUE];
    int count = 0;
};

namespace param
{

void add(Args &a, const char *name, const char *value);
const char *get(const Args &a, const char *name);
int get_int(const Args &a, const char *name);
float get_float(const Args &a, const char *name);
const char *type_name(ParamType type);

} // namespace param

enum class CommandId : int
{
#define CMD(id, name, help, params, action) id,
#include "command.def"
#undef CMD
    COUNT
};

#endif
