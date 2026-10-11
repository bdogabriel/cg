#include "face_batch.h"
#include <cmath>

namespace face
{

void update_corner_starts(FaceBatch &batch)
{
    int running = 0;
    for (int i = 0; i < batch.faceCount; i++)
    {
        batch.faceCornerStarts[i] = running;
        running += batch.faceCornerCounts[i];
    }
}

namespace
{

Vec4 compute_normal(const Vec4 *loop, int count)
{
    Vec4 normal = {0, 0, 0, 1};
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        normal.x += (loop[i].y - loop[j].y) * (loop[i].z + loop[j].z);
        normal.y += (loop[i].z - loop[j].z) * (loop[i].x + loop[j].x);
        normal.z += (loop[i].x - loop[j].x) * (loop[i].y + loop[j].y);
    }
    return normal;
}

void drop_dominant_axis(const Vec4 *loop, int count, float *out2d, int &axis_x, int &axis_y)
{
    Vec4 normal = compute_normal(loop, count);
    float abs_x = std::abs(normal.x);
    float abs_y = std::abs(normal.y);
    float abs_z = std::abs(normal.z);

    if (abs_x >= abs_y && abs_x >= abs_z)
    {
        axis_x = 1;
        axis_y = 2;
    }
    else if (abs_y >= abs_x && abs_y >= abs_z)
    {
        axis_x = 0;
        axis_y = 2;
    }
    else
    {
        axis_x = 0;
        axis_y = 1;
    }

    for (int i = 0; i < count; i++)
    {
        out2d[i * 2] = (&loop[i].x)[axis_x];
        out2d[i * 2 + 1] = (&loop[i].x)[axis_y];
    }
}

float compute_signed_area_2d(const float *poly2d, int count)
{
    float area = 0;
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        area += poly2d[i * 2] * poly2d[j * 2 + 1];
        area -= poly2d[j * 2] * poly2d[i * 2 + 1];
    }
    return area * 0.5f;
}

bool is_point_in_triangle_2d(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
    float d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
    float d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

bool is_ear(const float *poly2d, int count, const int *remaining, int i)
{
    int prev = (i + count - 1) % count;
    int next = (i + 1) % count;

    int a = remaining[prev];
    int b = remaining[i];
    int c = remaining[next];

    float ax = poly2d[a * 2], ay = poly2d[a * 2 + 1];
    float bx = poly2d[b * 2], by = poly2d[b * 2 + 1];
    float cx = poly2d[c * 2], cy = poly2d[c * 2 + 1];

    float cross = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    if (cross <= 0)
        return false;

    for (int j = 0; j < count; j++)
    {
        if (j == prev || j == i || j == next)
            continue;

        int v = remaining[j];
        float vx = poly2d[v * 2], vy = poly2d[v * 2 + 1];

        if (is_point_in_triangle_2d(vx, vy, ax, ay, bx, by, cx, cy))
            return false;
    }

    return true;
}

} // namespace

int triangulate(const Vec4 *loop, int count, int *outTris)
{
    if (count < 3)
        return 0;

    if (count == 3)
    {
        outTris[0] = 0;
        outTris[1] = 1;
        outTris[2] = 2;
        return 1;
    }

    float poly2d[128];
    int axis_x, axis_y;
    drop_dominant_axis(loop, count, poly2d, axis_x, axis_y);

    float area = compute_signed_area_2d(poly2d, count);

    int remaining[64];
    for (int i = 0; i < count; i++)
        remaining[i] = i;

    if (area < 0)
    {
        for (int i = 0; i < count / 2; i++)
        {
            int j = count - 1 - i;
            int tmp = remaining[i];
            remaining[i] = remaining[j];
            remaining[j] = tmp;
        }
    }

    int tri_count = 0;
    int remaining_count = count;

    while (remaining_count > 3)
    {
        bool found_ear = false;

        for (int i = 0; i < remaining_count; i++)
        {
            if (is_ear(poly2d, remaining_count, remaining, i))
            {
                int prev = (i + remaining_count - 1) % remaining_count;
                int next = (i + 1) % remaining_count;

                outTris[tri_count * 3 + 0] = remaining[prev];
                outTris[tri_count * 3 + 1] = remaining[i];
                outTris[tri_count * 3 + 2] = remaining[next];
                tri_count++;

                for (int j = i; j < remaining_count - 1; j++)
                    remaining[j] = remaining[j + 1];
                remaining_count--;

                found_ear = true;
                break;
            }
        }

        if (!found_ear)
        {
            for (int i = 1; i < remaining_count - 1; i++)
            {
                outTris[tri_count * 3 + 0] = remaining[0];
                outTris[tri_count * 3 + 1] = remaining[i];
                outTris[tri_count * 3 + 2] = remaining[i + 1];
                tri_count++;
            }
            break;
        }
    }

    if (remaining_count == 3)
    {
        outTris[tri_count * 3 + 0] = remaining[0];
        outTris[tri_count * 3 + 1] = remaining[1];
        outTris[tri_count * 3 + 2] = remaining[2];
        tri_count++;
    }

    return tri_count;
}

} // namespace face
