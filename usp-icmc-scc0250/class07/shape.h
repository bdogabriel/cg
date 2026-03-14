#pragma once

#include <array>

constexpr std::array<float, 16> SQUARE_V4 = {
     1, -1, 0, 1,
     1,  1, 0, 1,
    -1, -1, 0, 1,
    -1,  1, 0, 1
};

constexpr std::array<unsigned int, 6> SQUARE_IDX = {
    0, 1, 2,  2, 1, 3
};

constexpr std::array<float, 32> CUBE_V4 = {
     1, -1,  1, 1,  // 0 front-bottom-right
     1,  1,  1, 1,  // 1 front-top-right
    -1, -1,  1, 1,  // 2 front-bottom-left
    -1,  1,  1, 1,  // 3 front-top-left
     1, -1, -1, 1,  // 4 back-bottom-right
     1,  1, -1, 1,  // 5 back-top-right
    -1, -1, -1, 1,  // 6 back-bottom-left
    -1,  1, -1, 1,  // 7 back-top-left
};

constexpr std::array<unsigned int, 36> CUBE_IDX = {
    0, 1, 2,  2, 1, 3,  // front
    6, 7, 4,  4, 7, 5,  // back
    1, 5, 3,  3, 5, 7,  // top
    0, 2, 4,  4, 2, 6,  // bottom
    0, 4, 1,  1, 4, 5,  // right
    2, 3, 6,  6, 3, 7,  // left
};
