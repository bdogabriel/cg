#include "dispatch.h"
#include "binding.h"
#include "command.h"
#include "prompt.h"

namespace input
{

void dispatch(Editor &e)
{
    const Keyboard &kb = e.keyboard;

    if (cmd::is_exec(e))
    {
        if (kb.keyStates[(int)Key::ESC] == KeyState::JUST_PRESSED)
        {
            e.cmd = cmd::find("camera");
            return;
        }
        prompt::handle_input(e.prompt, kb);
        if (kb.keyStates[(int)Key::ENTER] == KeyState::JUST_PRESSED)
        {
            e.cmd->action(e, true);
        }
        return;
    }

    CommandCall calls[(int)Key::COUNT];
    int n = binding::resolve(kb, calls, (int)Key::COUNT);
    for (int i = 0; i < n; i++)
    {
        cmd::run(e, calls[i].cmd, calls[i].args, calls[i].justPressed);
    }
}

} // namespace input
