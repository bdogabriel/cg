#ifndef CAMERA_H
#define CAMERA_H

#include "mat4.h"

struct Camera
{
    Vec4 position = {0, 0, 3, 1};
    float yaw = 0.0f;           // radians, around +Y
    float pitch = 0.0f;         // radians, around +X
    float fovY = 1.0471975512f; // 60 degrees (pi/3)
    float near = 0.1f;
    float far = 100.0f;
};

struct WorldBounds
{
    float minX, maxX;
    float minY, maxY; // floor .. sky ceiling
    float minZ, maxZ;
};

namespace camera
{
inline constexpr WorldBounds WORLD_BOUNDS = {-40.0f, 40.0f, 0.0f, 40.0f, -40.0f, 40.0f};

Vec4 forward(const Camera &c);
Vec4 right(const Camera &c);
Vec4 up(const Camera &c);
Mat4 view(const Camera &c);
Mat4 view_rotation(const Camera &c);
Mat4 projection(const Camera &c, float aspect);
void clamp_position(Camera &c, const WorldBounds &b);
} // namespace camera

#endif
