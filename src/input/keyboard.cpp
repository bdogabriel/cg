#include "keyboard.h"

namespace keyboard
{
// reverse map: glfw key code -> Key enum value. gcc rejects the designated-
// initializer form for this array ("non-trivial designated initializers not
// supported"), so the mapping is filled in the constexpr ctor below.
struct KeyMap
{
    Key data[GLFW_KEY_LAST + 1];
    constexpr KeyMap() : data{}
    {
        data[GLFW_KEY_ESCAPE] = Key::ESC;
        data[GLFW_KEY_T] = Key::T;
        data[GLFW_KEY_R] = Key::R;
        data[GLFW_KEY_S] = Key::S;
        data[GLFW_KEY_A] = Key::A;
        data[GLFW_KEY_F] = Key::F;
        data[GLFW_KEY_W] = Key::W;
        data[GLFW_KEY_X] = Key::X;
        data[GLFW_KEY_Y] = Key::Y;
        data[GLFW_KEY_Z] = Key::Z;
        data[GLFW_KEY_N] = Key::N;
        data[GLFW_KEY_P] = Key::P;
        data[GLFW_KEY_E] = Key::E;
        data[GLFW_KEY_M] = Key::M;
        data[GLFW_KEY_D] = Key::D;
        data[GLFW_KEY_O] = Key::O;
        data[GLFW_KEY_C] = Key::C;
        data[GLFW_KEY_SPACE] = Key::SPACE;
        data[GLFW_KEY_H] = Key::H;
        data[GLFW_KEY_L] = Key::L;
        data[GLFW_KEY_J] = Key::J;
        data[GLFW_KEY_K] = Key::K;
        data[GLFW_KEY_U] = Key::U;
        data[GLFW_KEY_SEMICOLON] = Key::SEMICOLON;
        data[GLFW_KEY_ENTER] = Key::ENTER;
        data[GLFW_KEY_BACKSPACE] = Key::BACKSPACE;
    }
};

static const KeyMap keyOfGlfw{};

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int glfwMods)
{
    (void)scancode; // glfw mandates this callback parameter
    Keyboard *kb = static_cast<Keyboard *>(glfwGetWindowUserPointer(window));
    Key k = keyOfGlfw.data[key];

    if (k == Key::NONE)
    {
        return;
    }
    if (action == GLFW_PRESS)
    {
        kb->keyStates[(int)k] = KeyState::JUST_PRESSED;
    }
    else if (action == GLFW_RELEASE)
    {
        kb->keyStates[(int)k] = KeyState::RELEASED;
    }

    kb->mods = (uint8_t)(glfwMods & mods::ALL);
}

static void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    Keyboard *kb = static_cast<Keyboard *>(glfwGetWindowUserPointer(window));
    if (codepoint < 32 || codepoint >= 127)
    {
        return;
    }
    if (kb->charCount < (int)(sizeof(kb->chars) - 1))
    {
        kb->chars[kb->charCount++] = (char)codepoint;
    }
}

void setup(Keyboard &kb, GLFWwindow *window)
{
    glfwSetWindowUserPointer(window, &kb);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
}

void reset(Keyboard &kb)
{
    for (auto &k : kb.keyStates)
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
} // namespace keyboard
