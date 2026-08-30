#include "scene.h"
#include "trs.h"
#include "wavefront.h"

static Ref load_and_position(Buffer &buf, Editor &e, const char *path, float x)
{
    if (!wfront::load(buf, e, path, false))
    {
        return -1;
    }
    Ref ref = e.selectedRef;
    e.transforms[ref].tx = x;
    e.transforms[ref].sx = 0.5;
    e.transforms[ref].sy = 0.5;
    e.transforms[ref].sz = 0.5;
    buffer::recompose_model(buf, ref, trs::compose(e.transforms[ref]));
    return ref;
}

bool scene::load_models(Buffer &buf, Editor &e)
{
    const float spacing = 0.35f;
    const float startX = -0.8f;

    Ref bowRef = load_and_position(buf, e, "models/bow.obj", startX);
    Ref shieldRef = load_and_position(buf, e, "models/shield.obj", startX + spacing);
    Ref swordRef = load_and_position(buf, e, "models/sword.obj", startX + spacing * 2);
    Ref grimoireRef = load_and_position(buf, e, "models/grimoire.obj", startX + spacing * 3);
    Ref staffRef = load_and_position(buf, e, "models/staff.obj", startX + spacing * 4);

    if (bowRef < 0 || shieldRef < 0 || swordRef < 0 || grimoireRef < 0 || staffRef < 0)
    {
        return false;
    }

    printf("Controls:\n");
    printf("  n/p       - Next/previous object (or face when target=face)\n");
    printf("  f/o       - Face/object target\n");
    printf("  t/r/s/a   - Translate/Rotate/Scale/Shear mode\n");
    printf("  e         - Extrude (face target)\n");
    printf("  d/m       - Delete/Merge object\n");
    printf("  h/j/k/l   - Transform along X/Y (hold)\n");
    printf("  ctrl+u/d  - Transform along Z (hold)\n");
    printf("  x/y/z     - Lock axis\n");
    printf("  space     - Toggle face selection\n");
    printf("  c         - Clear face selection\n");
    printf("  w         - Toggle wireframe\n");
    printf("  u         - Undo\n");
    printf("  shift+r   - Reset rotation\n");
    printf("  shift+t   - Reset translation\n");
    printf("  esc       - No command\n\n");

    // load leaves selectedRef on the last-loaded ref and the editor ctor
    // defaults it to reserved slot 0, so reset to the bow explicitly
    e.selectedRef = bowRef;

    return true;
}
