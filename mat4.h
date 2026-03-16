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

    [[nodiscard]] Mat4 operator*(const Mat4 &B) const noexcept
    {
        Mat4 R;

        for (int i = 0; i < 4; i++)
        {
            __m128 b = B.col[i];

            __m128 b0 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 0, 0, 0));
            __m128 b1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 1, 1, 1));
            __m128 b2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 2, 2, 2));
            __m128 b3 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 3, 3, 3));

            R.col[i] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col[0], b0), _mm_mul_ps(col[1], b1)),
                                  _mm_add_ps(_mm_mul_ps(col[2], b2), _mm_mul_ps(col[3], b3)));
        }

        return R;
    }

    Mat4 &operator*=(const Mat4 &B) noexcept
    {
        *this = *this * B;
        return *this;
    }
};
