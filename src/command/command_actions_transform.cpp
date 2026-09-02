#include "command_actions.h"
#include "editor.h"
#include "face_edit.h"
#include "meshbatch.h"
#include "session.h"
#include "trs.h"

namespace cmd
{

AxisLock axis_lock(const char *s)
{
    if (!s)
    {
        return AxisLock::None;
    }
    if (s[0] == 'x')
    {
        return AxisLock::X;
    }
    if (s[0] == 'y')
    {
        return AxisLock::Y;
    }
    if (s[0] == 'z')
    {
        return AxisLock::Z;
    }
    return AxisLock::None;
}

const char *const axis_choices[] = {"x", "y", "z", nullptr};

const ParamSpec params_axis_step[] = {
    {"axis", ParamType::STRING, axis_choices, "axis to act on", true, nullptr},
    {"step", ParamType::INT, nullptr, "signed step size (negative = reverse)", true, nullptr},
    {nullptr, ParamType::STRING, nullptr, "", false, nullptr},
};

static bool transform_prepare(const Editor &e, const Args &a, AxisLock &axis, int &step, bool &isFace)
{
    const char *ax = a.get("axis");
    if (!ax)
    {
        return false;
    }
    axis = axis_lock(ax);
    if (axis == AxisLock::None)
    {
        return false;
    }
    isFace = (e.target == Target::Face);
    if (isFace && e.selectedFaceCount == 0)
    {
        return false;
    }
    step = a.getInt("step");
    return true;
}

using FaceDeltaFn = Mat4 (*)(AxisLock, float);
using ObjectApplyFn = void (*)(Editor &, AxisLock, float);

static void run_transform(Editor &e, const Args &a, bool snapshot, FaceDeltaFn faceFn, ObjectApplyFn objFn)
{
    AxisLock axis;
    int step;
    bool isFace;
    if (!transform_prepare(e, a, axis, step, isFace))
    {
        return;
    }
    if (snapshot)
    {
        editor::push_undo(e);
    }
    float amount = step * e.keySensitivity;
    if (isFace)
    {
        face::transform(e.meshBatch, e.selectedRef, e.selectedFaces, e.selectedFaceCount, faceFn(axis, amount));
    }
    else
    {
        objFn(e, axis, amount);
        mesh::compose_model(e.meshBatch, e.selectedRef, trs::compose(e.transforms[e.selectedRef]));
    }
}

static Mat4 translate_face_delta(AxisLock axis, float amount)
{
    switch (axis)
    {
    case AxisLock::X:
        return trs::translation(amount, 0, 0);
    case AxisLock::Y:
        return trs::translation(0, amount, 0);
    case AxisLock::Z:
        return trs::translation(0, 0, amount);
    default:
        return mat4::IDENTITY;
    }
}

static void translate_object_apply(Editor &e, AxisLock axis, float amount)
{
    TRS &t = e.transforms[e.selectedRef];
    if (!e.locked)
    {
        switch (axis)
        {
        case AxisLock::X:
            trs::translate(t, amount, 0, 0);
            break;
        case AxisLock::Y:
            trs::translate(t, 0, amount, 0);
            break;
        case AxisLock::Z:
            trs::translate(t, 0, 0, amount);
            break;
        case AxisLock::None:
            break;
        }
    }
    else
    {
        switch (axis)
        {
        case AxisLock::X:
            trs::translate_local(t, amount, 0, 0);
            break;
        case AxisLock::Y:
            trs::translate_local(t, 0, amount, 0);
            break;
        case AxisLock::Z:
            trs::translate_local(t, 0, 0, amount);
            break;
        case AxisLock::None:
            break;
        }
    }
}

void do_translate(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)s;
    run_transform(e, a, snapshot, translate_face_delta, translate_object_apply);
}

static Mat4 rotate_face_delta(AxisLock axis, float amount)
{
    switch (axis)
    {
    case AxisLock::X:
        return trs::rotation_y(amount);
    case AxisLock::Y:
        return trs::rotation_x(-amount);
    case AxisLock::Z:
        return trs::rotation_z(amount);
    default:
        return mat4::IDENTITY;
    }
}

static void rotate_object_apply(Editor &e, AxisLock axis, float amount)
{
    TRS &t = e.transforms[e.selectedRef];
    if (!e.locked)
    {
        switch (axis)
        {
        // invert to make keybindings intuitive
        case AxisLock::X:
            trs::rotate_y(t, amount);
            break;
        case AxisLock::Y:
            trs::rotate_x(t, -amount);
            break;
        case AxisLock::Z:
            trs::rotate_z(t, amount);
            break;
        case AxisLock::None:
            break;
        }
    }
    else
    {
        switch (axis)
        {
        case AxisLock::X:
            trs::rotate_y_local(t, amount);
            break;
        case AxisLock::Y:
            trs::rotate_x_local(t, -amount);
            break;
        case AxisLock::Z:
            trs::rotate_z_local(t, amount);
            break;
        case AxisLock::None:
            break;
        }
    }
}

void do_rotate(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)s;
    run_transform(e, a, snapshot, rotate_face_delta, rotate_object_apply);
}

static Mat4 scale_face_delta(AxisLock axis, float amount)
{
    float s = 1.0f + amount;
    switch (axis)
    {
    case AxisLock::X:
        return trs::scaling(s, 1, 1);
    case AxisLock::Y:
        return trs::scaling(1, s, 1);
    case AxisLock::Z:
        return trs::scaling(1, 1, s);
    default:
        return mat4::IDENTITY;
    }
}

static void scale_object_apply(Editor &e, AxisLock axis, float amount)
{
    TRS &t = e.transforms[e.selectedRef];
    float s = 1.0f + amount;
    switch (axis)
    {
    case AxisLock::X:
        trs::scale(t, s, 1, 1);
        break;
    case AxisLock::Y:
        trs::scale(t, 1, s, 1);
        break;
    case AxisLock::Z:
        trs::scale(t, 1, 1, s);
        break;
    case AxisLock::None:
        break;
    }
}

void do_scale(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)s;
    run_transform(e, a, snapshot, scale_face_delta, scale_object_apply);
}

static Mat4 shear_face_delta(AxisLock axis, float amount)
{
    switch (axis)
    {
    case AxisLock::X:
        return trs::shearing(amount, 0, 0, 0, 0, 0);
    case AxisLock::Y:
        return trs::shearing(0, 0, 0, amount, 0, 0);
    case AxisLock::Z:
        return trs::shearing(0, 0, 0, 0, 0, amount);
    default:
        return mat4::IDENTITY;
    }
}

static void shear_object_apply(Editor &e, AxisLock axis, float amount)
{
    TRS &t = e.transforms[e.selectedRef];
    switch (axis)
    {
    case AxisLock::X:
        trs::shear(t, amount, 0, 0, 0, 0, 0);
        break;
    case AxisLock::Y:
        trs::shear(t, 0, 0, 0, amount, 0, 0);
        break;
    case AxisLock::Z:
        trs::shear(t, 0, 0, 0, 0, 0, amount);
        break;
    case AxisLock::None:
        break;
    }
}

void do_shear(Editor &e, Session &s, const Args &a, bool snapshot)
{
    (void)s;
    run_transform(e, a, snapshot, shear_face_delta, shear_object_apply);
}

} // namespace cmd
