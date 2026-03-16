#pragma once

#include "trs.h"
#include "types.h"
#include <GL/glew.h>
#include <array>
#include <vector>

struct GeometryBuffer
{
    std::vector<float> v;
    std::vector<unsigned int> i;
    GLuint vao = 0, vbo = 0, ebo = 0;

    void upload(GLuint idxVertex);
    void update() const;
    void draw(GLenum primitive) const;
    void free();

    ~GeometryBuffer()
    {
        if (vao)
        {
            free();
        }
    }
};

namespace geometry
{
GeometryBuffer cube();
GeometryBuffer axis_x();
GeometryBuffer axis_y();
GeometryBuffer axis_z();

constexpr std::array<float, 16> SQUARE_V = {1,  -1, 0, 1, //
                                            1,  1,  0, 1, //
                                            -1, -1, 0, 1, //
                                            -1, 1,  0, 1};

constexpr std::array<unsigned int, 6> SQUARE_I = {0, 1, 2, //
                                                  2, 1, 3};

constexpr std::array<float, 32> CUBE_V = {
    1,  -1, 1,  1, //
    1,  1,  1,  1, //
    -1, -1, 1,  1, //
    -1, 1,  1,  1, //
    1,  -1, -1, 1, //
    1,  1,  -1, 1, //
    -1, -1, -1, 1, //
    -1, 1,  -1, 1,
};

constexpr std::array<unsigned int, 36> CUBE_I = {
    0, 1, 2, //
    2, 1, 3, //
    6, 7, 4, //
    4, 7, 5, //
    1, 5, 3, //
    3, 5, 7, //
    0, 2, 4, //
    4, 2, 6, //
    0, 4, 1, //
    1, 4, 5, //
    2, 3, 6, //
    6, 3, 7,
};
} // namespace geometry
