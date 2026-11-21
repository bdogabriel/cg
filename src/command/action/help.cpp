#include "binding.h"
#include "command.h"
#include "editor.h"
#include <cstdio>
#include <string.h>

namespace cmd
{

namespace
{

void print_param(const Param &ps)
{
    printf("    %-8s <%s>", ps.name, param::type_name(ps.type));
    if (ps.required)
    {
        printf(" (required)");
    }
    else if (ps.defaultVal && ps.defaultVal[0])
    {
        printf(" = %s", ps.defaultVal);
    }
    if (ps.help && ps.help[0])
    {
        printf("  - %s", ps.help);
    }
    if (ps.choices)
    {
        printf("  [");
        for (int ch = 0; ps.choices[ch]; ch++)
        {
            if (ch > 0)
            {
                printf("|");
            }
            printf("%s", ps.choices[ch]);
        }
        printf("]");
    }
    printf("\n");
}

void print_usage(const Command &c)
{
    printf("usage: %s", c.name);
    for (int p = 0; p < c.paramCount; p++)
    {
        printf(c.params[p].required ? " <%s>" : " [%s]", c.params[p].name);
    }
    printf("\n");
}

} // namespace

void print_command_help(const Command &c)
{
    print_usage(c);
    printf("%s\n", c.help);
    for (int p = 0; p < c.paramCount; p++)
    {
        print_param(c.params[p]);
    }
}

namespace
{

void print_all_commands_help()
{
    for (int i = 0; i < count(); i++)
    {
        print_command_help(*at(i));
        printf("\n");
    }
}

void print_all_bindings_help()
{
    for (int i = 0; i < binding::count(); i++)
    {
        const Binding *b = binding::at(i);
        char chord[64];
        binding::chord_name(*b, chord, sizeof(chord));

        printf("%-8s -> %s", chord, b->cmd->name);
        if (b->args && b->args[0])
        {
            printf(" %s", b->args);
        }
        if (b->oneShot)
        {
            printf("  [oneshot]");
        }
        printf("\n");
    }
}

} // namespace

void do_help(Editor &e, bool)
{
    const char *topic = param::get(e.args, "topic");
    if (strcmp(topic, "commands") == 0)
    {
        print_all_commands_help();
    }
    else if (strcmp(topic, "bindings") == 0)
    {
        print_all_bindings_help();
    }
    else
    {
        const Command *c = find(topic);
        if (c)
        {
            print_command_help(*c);
        }
        else
        {
            printf("E: unknown command '%s'\n", topic);
        }
    }
}

} // namespace cmd
