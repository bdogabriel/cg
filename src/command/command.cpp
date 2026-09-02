#include "command.h"
#include "command_actions.h"
#include "editor.h"
#include "session.h"
#include <cstdio>

namespace cmd
{

static const Command registry[] = {
#define CMD(id, name, help, params, action) {name, help, params, action},
#include "command.def"
#undef CMD
};

const char *const cmd_choices[] = {
#define CMD(id, name, help, params, action) name,
#include "command.def"
#undef CMD
    nullptr};

const Command *find(const char *name)
{
    for (int i = 0; i < (int)CommandId::COUNT; i++)
    {
        if (strcmp(registry[i].name, name) == 0)
        {
            return &registry[i];
        }
    }
    return nullptr;
}

const Command *get(CommandId id)
{
    return &registry[(int)id];
}

void do_list(Editor &, Session &, const Args &, bool)
{
    for (int i = 0; i < (int)CommandId::COUNT; i++)
    {
        printf("%s - %s\n", registry[i].name, registry[i].help);
    }
}

static char tokenbuf[512];

// TODO: extract/split
bool parse_args(const char *argsStr, const ParamSpec *params, Args &out)
{
    out.count = 0;

    int len = strlen(argsStr);
    if (len >= (int)sizeof(tokenbuf))
    {
        printf("E: argument string too long (max %d)\n", (int)sizeof(tokenbuf) - 1);
        return false;
    }
    memcpy(tokenbuf, argsStr, len);
    tokenbuf[len] = '\0';

    char *tokens[64];
    int ntok = 0;
    bool in_token = false;
    int start = 0;
    for (int i = 0; i <= len; i++)
    {
        bool is_ws = (tokenbuf[i] == ' ' || tokenbuf[i] == '\t' || tokenbuf[i] == '\0');
        if (is_ws)
        {
            if (in_token)
            {
                tokenbuf[i] = '\0';
                tokens[ntok++] = tokenbuf + start;
                in_token = false;
            }
        }
        else
        {
            if (!in_token)
            {
                start = i;
                in_token = true;
            }
        }
    }

    auto find_param = [&](const char *name) -> int {
        for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
        {
            if (strcmp(params[i].name, name) == 0)
            {
                return i;
            }
        }
        return -1;
    };

    auto already_set = [&](const char *name) -> bool {
        for (int i = 0; i < out.count; i++)
        {
            if (strcmp(out.names[i], name) == 0)
            {
                return true;
            }
        }
        return false;
    };

    for (int t = 0; t < ntok; t++)
    {
        char *eq = strchr(tokens[t], '=');
        if (eq != nullptr)
        {
            *eq = '\0';
            const char *name = tokens[t];
            const char *value = eq + 1;

            int idx = find_param(name);
            if (idx < 0)
            {
                printf("E: unknown parameter '%s'\n", name);
                return false;
            }
            const ParamSpec &p = params[idx];

            if (already_set(p.name))
            {
                printf("E: parameter '%s' specified more than once\n", p.name);
                return false;
            }

            if (p.type == ParamType::INT)
            {
                char *end = nullptr;
                long v = strtol(value, &end, 10);
                if (end == value || (*end != '\0'))
                {
                    printf("E: '%s' expects an integer\n", p.name);
                    return false;
                }
                (void)v;
            }
            else if (p.type == ParamType::STRING && p.choices != nullptr)
            {
                bool found = false;
                for (int i = 0; p.choices[i] != nullptr; i++)
                {
                    if (strcmp(p.choices[i], value) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    printf("E: bad value for '%s' (expected one of ", p.name);
                    for (int i = 0; p.choices[i] != nullptr; i++)
                    {
                        if (i > 0)
                        {
                            printf("|");
                        }
                        printf("%s", p.choices[i]);
                    }
                    printf(")\n");
                    return false;
                }
            }

            out.set(out.count, p.name, value);
            out.count++;
        }
        else
        {
            int idx = -1;
            for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
            {
                if (!already_set(params[i].name))
                {
                    idx = i;
                    break;
                }
            }
            if (idx < 0)
            {
                printf("E: too many positional arguments\n");
                return false;
            }
            const ParamSpec &p = params[idx];
            const char *value = tokens[t];

            if (p.type == ParamType::INT)
            {
                char *end = nullptr;
                long v = strtol(value, &end, 10);
                if (end == value || (*end != '\0'))
                {
                    printf("E: '%s' expects an integer\n", p.name);
                    return false;
                }
                (void)v;
            }
            else if (p.type == ParamType::STRING && p.choices != nullptr)
            {
                bool found = false;
                for (int i = 0; p.choices[i] != nullptr; i++)
                {
                    if (strcmp(p.choices[i], value) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    printf("E: bad value for '%s' (expected one of ", p.name);
                    for (int i = 0; p.choices[i] != nullptr; i++)
                    {
                        if (i > 0)
                        {
                            printf("|");
                        }
                        printf("%s", p.choices[i]);
                    }
                    printf(")\n");
                    return false;
                }
            }

            out.set(out.count, p.name, value);
            out.count++;
        }
    }

    for (int i = 0; params != nullptr && params[i].name != nullptr; i++)
    {
        if (already_set(params[i].name))
        {
            continue;
        }
        if (params[i].required)
        {
            printf("E: missing required parameter '%s'\n", params[i].name);
            return false;
        }
        out.set(out.count, params[i].name, (params[i].defaultVal != nullptr) ? params[i].defaultVal : "");
        out.count++;
    }

    return true;
}

void run(Editor &e, Session &s, const Command *c, const char *argsStr, bool snapshot)
{
    if (!parse_args(argsStr, c->params, s.args))
    {
        return;
    }
    c->action(e, s, s.args, snapshot);
}

} // namespace cmd
