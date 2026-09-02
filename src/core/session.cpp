#include "session.h"
#include "binding.h"

namespace session
{

void setup(Session &s, Keyboard &keyboard)
{
    s.keyboard = &keyboard;
}

void process_input(Editor &e, Session &s)
{
    const Keyboard &kb = *s.keyboard;

    if (session::is_cmd_exec(s))
    {
        if (kb.keyStates[(int)Key::ESC] == KeyState::JUST_PRESSED)
        {
            s.cmd = cmd::get(CommandId::none);
            return;
        }
        prompt::handle_input(s.prompt, kb);
        if (kb.keyStates[(int)Key::ENTER] == KeyState::JUST_PRESSED)
        {
            s.cmd->action(e, s, s.args, true);
        }
        return;
    }

    CommandCall calls[(int)Key::COUNT];
    int n = binding::resolve(kb, calls, (int)Key::COUNT);
    for (int i = 0; i < n; i++)
    {
        cmd::run(e, s, calls[i].cmd, calls[i].args, calls[i].snapshot);
    }
}

} // namespace session
