#include "scene.h"
#include "trs.h"
#include <cstdio>

static Ref load_and_position(DrawBuffer &buf, EditorState &state, const std::filesystem::path &path, float x)
{
    if (!obj_load(buf, state, path.string(), false))
    {
        return -1;
    }
    Ref ref = state.selectedRef;
    buf.transforms[ref].tx = x;
    buf.transforms[ref].sx = 0.5;
    buf.transforms[ref].sy = 0.5;
    buf.transforms[ref].sz = 0.5;
    buf.models[ref] = trs::compose(buf.transforms[ref]);
    return ref;
}

bool scene::load_models(DrawBuffer &buf, EditorState &state, const std::filesystem::path &modelsDir)
{
    const float spacing = 0.35f;
    const float startX = -0.8f;

    Ref bowRef = load_and_position(buf, state, modelsDir / "bow.obj", startX);
    Ref shieldRef = load_and_position(buf, state, modelsDir / "shield.obj", startX + spacing);
    Ref swordRef = load_and_position(buf, state, modelsDir / "sword.obj", startX + spacing * 2);
    Ref grimoireRef = load_and_position(buf, state, modelsDir / "grimoire.obj", startX + spacing * 3);
    Ref staffRef = load_and_position(buf, state, modelsDir / "staff.obj", startX + spacing * 4);

    if (bowRef < 0 || shieldRef < 0 || swordRef < 0 || grimoireRef < 0 || staffRef < 0)
    {
        return false;
    }

    buf.update();

    printf("Controls:\n");
    printf("  n/p     - Next/previous object\n");
    printf("  t       - Translate mode, then use H/J/K/L\n");
    printf("  r       - Rotate mode, then use H/J/K/L\n");
    printf("  s       - Scale mode, then use H/J/K/L\n");
    printf("  w       - Toggle wireframe\n");
    printf("  Esc     - Return to normal mode (no transformation selected)\n");
    printf("  x/y/z   - Lock transform to axis (inside a mode)\n\n");

    state.selectedRef = bowRef;

    return true;
}
