#ifndef CAMERA_H
#define CAMERA_H

#include "mat4.h"

struct Camera
{
    Vec4 position = {0, 0, 3, 1};
    float yaw = 0.0f;           // radians, around +Y
    float pitch = 0.0f;         // radians, around +X
    float fovY = 0.7853981634f; // 45 degrees
    float near = 0.1f;
    float far = 100.0f;
};

namespace camera
{
Vec4 forward(const Camera &c);
Vec4 right(const Camera &c);
Vec4 up(const Camera &c);
Mat4 view(const Camera &c);
Mat4 projection(const Camera &c, float aspect);
} // namespace camera

#endif
