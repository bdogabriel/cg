#include "keyboard.h"
#include "window.h"

#include "glfw_internal.h"

#include <GLFW/glfw3.h>

namespace keyboard
{

namespace
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

const KeyMap keyOfGlfw{};

void key_callback(GLFWwindow *handle, int key, int scancode, int action, int glfwMods)
{
    (void)handle;
    (void)scancode;

    Keyboard *kb = glfwplat::state().keyboard;
    if (!kb || key < 0 || key > GLFW_KEY_LAST)
    {
        return;
    }

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

    uint8_t mods = mods::NONE;
    if (glfwMods & GLFW_MOD_SHIFT)
    {
        mods |= mods::SHIFT;
    }
    if (glfwMods & GLFW_MOD_CONTROL)
    {
        mods |= mods::CTRL;
    }
    if (glfwMods & GLFW_MOD_ALT)
    {
        mods |= mods::ALT;
    }
    kb->mods = mods;
}

void char_callback(GLFWwindow *handle, unsigned int codepoint)
{
    (void)handle;

    Keyboard *kb = glfwplat::state().keyboard;
    if (!kb)
    {
        return;
    }
    if (codepoint < 32 || codepoint >= 127)
    {
        return;
    }
    if (kb->charCount < (int)(sizeof(kb->chars) - 1))
    {
        kb->chars[kb->charCount++] = (char)codepoint;
    }
}

} // namespace

void setup(Keyboard &kb, Window &w)
{
    glfwplat::state().keyboard = &kb;

    GLFWwindow *handle = static_cast<GLFWwindow *>(window::native(w));
    if (!handle)
    {
        return;
    }

    glfwSetKeyCallback(handle, key_callback);
    glfwSetCharCallback(handle, char_callback);
}

} // namespace keyboard
