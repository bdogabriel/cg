#pragma once

#include <xmmintrin.h>

// column-major + broadcast scalars
struct alignas(16) Mat4
{
    __m128 col[4];

    Mat4() noexcept = default;

    [[nodiscard]] static Mat4 identity() noexcept
    {
        Mat4 m;

        m.col[0] = _mm_set_ps(0, 0, 0, 1);
        m.col[1] = _mm_set_ps(0, 0, 1, 0);
        m.col[2] = _mm_set_ps(0, 1, 0, 0);
        m.col[3] = _mm_set_ps(1, 0, 0, 0);

        return m;
    }

    [[nodiscard]] float *data() noexcept
    {
        return reinterpret_cast<float *>(col);
    }

    [[nodiscard]] const float *data() const noexcept
    {
        return reinterpret_cast<const float *>(col);
    }

    [[nodiscard]] Mat4 operator*(const Mat4 &m) const noexcept
    {
        Mat4 r;

        for (int i = 0; i < 4; i++)
        {
            __m128 c = m.col[i];

            __m128 c0 = _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0));
            __m128 c1 = _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1));
            __m128 c2 = _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2));
            __m128 c3 = _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 3, 3, 3));

            r.col[i] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col[0], c0), _mm_mul_ps(col[1], c1)),
                                  _mm_add_ps(_mm_mul_ps(col[2], c2), _mm_mul_ps(col[3], c3)));
        }

        return r;
    }

    Mat4 &operator*=(const Mat4 &m) noexcept
    {
        *this = *this * m;
        return *this;
    }
};
