#include "camera.h"
#include "action.h"
#include "editor.h"
#include "session.h"

namespace cmd
{

static void camera_move(Editor &e, AxisLock axis, float step)
{
    constexpr float MOVE_SPEED = 0.1f;
    Vec4 dir;
    switch (axis)
    {
    case AxisLock::X:
        dir = camera::right(e.camera); // h/l strafe
        break;
    case AxisLock::Y:
        dir = camera::forward(e.camera); // j/k forward/backward (in/out)
        break;
    case AxisLock::Z:
        dir = camera::up(e.camera); // u/d up/down
        break;
    case AxisLock::None:
        return;
    }

    // j (step=-1) = out, k (step=+1) = in
    e.camera.position += dir * (step * MOVE_SPEED);
}

static void camera_rotate(Editor &e, AxisLock axis, float step)
{
    float rotSpeed = e.keySensitivity;
    switch (axis)
    {
    case AxisLock::X:
        e.camera.yaw += step * rotSpeed; // h/l
        break;
    case AxisLock::Y:
        e.camera.pitch += step * rotSpeed; // j/k
        break;
    case AxisLock::Z:
    case AxisLock::None:
        break;
    }
}

void do_camera(Editor &e, Session &s, const Args &a, bool)
{
    (void)s;
    AxisLock axis = axis_lock(a.get("axis"));
    if (axis == AxisLock::None)
    {
        return;
    }
    float step = (float)a.getInt("step");

    if (e.locked)
    {
        camera_rotate(e, axis, step);
    }
    else
    {
        camera_move(e, axis, step);
    }
}

} // namespace cmd
