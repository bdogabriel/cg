#include "prompt.h"
#include "keyboard.h"

namespace prompt
{

void reset(Prompt &p)
{
    p.cursor = 0;
    p.buffer[0] = '\0';
}

void handle_input(Prompt &p, const Keyboard &kb)
{
    if (kb.keyStates[(int)Key::BACKSPACE] == KeyState::JUST_PRESSED && p.cursor > 0)
    {
        p.buffer[--p.cursor] = '\0';
    }
    for (int i = 0; i < kb.charCount; i++)
    {
        char ch = kb.chars[i];
        if (ch >= 32 && ch <= 126 && p.cursor < (int)sizeof(p.buffer) - 1)
        {
            p.buffer[p.cursor++] = ch;
            p.buffer[p.cursor] = '\0';
        }
    }
}

} // namespace prompt
