#ifndef PROMPT_H
#define PROMPT_H

struct Keyboard;

struct Prompt
{
    char buffer[512] = {};
    int cursor = 0;
};

namespace prompt
{
void reset(Prompt &p);
void handle_input(Prompt &p, const Keyboard &kb);
} // namespace prompt

#endif // PROMPT_H
