#include "input.h"

namespace input
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
    }
};

static const KeyMap keyOfGlfw{};

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int glfwMods)
{
    (void)scancode; // glfw mandates this callback parameter
    Input *inp = static_cast<Input *>(glfwGetWindowUserPointer(window));
    Key k = keyOfGlfw.data[key];

    if (k == Key::NONE)
    {
        return;
    }
    if (action == GLFW_PRESS)
    {
        inp->keyStates[(int)k] = KeyState::JUST_PRESSED;
    }
    else if (action == GLFW_RELEASE)
    {
        inp->keyStates[(int)k] = KeyState::RELEASED;
    }

    inp->mods = (uint8_t)(glfwMods & mods::ALL);
}

void setup(Input &inp, GLFWwindow *window)
{
    glfwSetWindowUserPointer(window, &inp);
    glfwSetKeyCallback(window, key_callback);
}

void reset(Input &inp)
{
    for (auto &k : inp.keyStates)
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
}
} // namespace input
