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

Mat4 view_rotation(const Camera &c)
{
    Vec4 eye = {0, 0, 0, 1};
    return viewproj::look_at(eye, eye + forward(c), WORLD_UP);
}

Mat4 projection(const Camera &c, float aspect)
{
    return viewproj::perspective(c.fovY, aspect, c.near, c.far);
}

void clamp_position(Camera &c, const WorldBounds &b)
{
    if (c.position.x < b.minX)
    {
        c.position.x = b.minX;
    }
    if (c.position.x > b.maxX)
    {
        c.position.x = b.maxX;
    }
    if (c.position.y < b.minY)
    {
        c.position.y = b.minY;
    }
    if (c.position.y > b.maxY)
    {
        c.position.y = b.maxY;
    }
    if (c.position.z < b.minZ)
    {
        c.position.z = b.minZ;
    }
    if (c.position.z > b.maxZ)
    {
        c.position.z = b.maxZ;
    }
}

} // namespace camera
