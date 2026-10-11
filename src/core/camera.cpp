#include "camera.h"
#include "view_projection.h"

namespace camera
{

namespace
{

const Vec4 WORLD_UP = {0, 1, 0, 0};

} // namespace

Vec4 forward(const Camera &c)
{
    float cp = cosf(c.pitch);
    return {cp * sinf(c.yaw), sinf(c.pitch), -cp * cosf(c.yaw), 0};
}

Vec4 right(const Camera &c)
{
    return vec4::normalize(vec4::cross(forward(c), WORLD_UP));
}

Vec4 up(const Camera &c)
{
    return vec4::cross(right(c), forward(c));
}

Mat4 view(const Camera &c)
{
    return viewproj::look_at(c.position, c.position + forward(c), WORLD_UP);
}

Mat4 projection(const Camera &c, float aspect)
{
    return viewproj::perspective(c.fovY, aspect, c.near, c.far);
}

} // namespace camera
