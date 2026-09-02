#ifndef SESSION_H
#define SESSION_H

#include "command.h"
#include "prompt.h"

struct Editor;
struct Keyboard;

// TODO: decouple god struct
struct Session
{
    Keyboard *keyboard = nullptr;
    const Command *cmd = cmd::get(CommandId::none);
    Args args;
    Prompt prompt;
    bool shouldQuit = false;
    char currentFile[256] = {};
};

namespace session
{
void setup(Session &s, Keyboard &keyboard);
void process_input(Editor &e, Session &s);
inline bool is_cmd_exec(const Session &s)
{
    return s.cmd == cmd::get(CommandId::execute);
}
} // namespace session

#endif
