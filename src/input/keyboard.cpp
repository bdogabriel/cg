#include "keyboard.h"

namespace keyboard
{

void reset(Keyboard &kb)
{
    for (KeyState &k : kb.keyStates)
    {
        if (k == KeyState::RELEASED)
        {
            k = KeyState::UP;
        }
        else if (k == KeyState::JUST_PRESSED)
        {
            k = KeyState::DOWN;
        }
    }
    kb.charCount = 0;
}

const char *key_name(Key k)
{
    switch (k)
    {
    case Key::ESC:
        return "Esc";
    case Key::T:
        return "T";
    case Key::R:
        return "R";
    case Key::S:
        return "S";
    case Key::A:
        return "A";
    case Key::F:
        return "F";
    case Key::W:
        return "W";
    case Key::X:
        return "X";
    case Key::Y:
        return "Y";
    case Key::Z:
        return "Z";
    case Key::N:
        return "N";
    case Key::P:
        return "P";
    case Key::E:
        return "E";
    case Key::M:
        return "M";
    case Key::D:
        return "D";
    case Key::O:
        return "O";
    case Key::C:
        return "C";
    case Key::SPACE:
        return "Space";
    case Key::H:
        return "H";
    case Key::L:
        return "L";
    case Key::J:
        return "J";
    case Key::K:
        return "K";
    case Key::U:
        return "U";
    case Key::SEMICOLON:
        return ";";
    case Key::ENTER:
        return "Enter";
    case Key::BACKSPACE:
        return "Backspace";
    default:
        return "?";
    }
}

const char *mod_name(uint8_t mod)
{
    switch (mod)
    {
    case mods::CTRL:
        return "Ctrl";
    case mods::SHIFT:
        return "Shift";
    case mods::ALT:
        return "Alt";
    default:
        return "?";
    }
}

} // namespace keyboard
