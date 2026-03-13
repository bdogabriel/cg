#pragma once

#include <xmmintrin.h>

struct alignas(16) Mat4
{
    __m128 col[4];

    float *data()
    {
        return reinterpret_cast<float *>(col);
    }

    const float *data() const
    {
        return reinterpret_cast<const float *>(col);
    }

    // column-major + broadcast scalars
    inline Mat4 operator*(const Mat4 &B) const
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

    inline Mat4 &operator*=(const Mat4 &B)
    {
        Mat4 R = *this * B;
        *this = R;
        return *this;
    }
};
