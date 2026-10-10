#ifndef VIEW_PROJECTION_H
#define VIEW_PROJECTION_H

#include "mat4.h"

namespace viewproj
{
Mat4 perspective(float fovY, float aspect, float near, float far);
Mat4 look_at(const Vec4 &eye, const Vec4 &center, const Vec4 &up);
} // namespace viewproj

#endif
