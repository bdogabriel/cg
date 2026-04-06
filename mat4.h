#ifndef MAT4_H
#define MAT4_H

#include <cmath>
#include <cstring>
#include <xmmintrin.h>

// TODO: optimize inverse

struct Vec4
{
    float x, y, z, w;

    Vec4 operator+(const Vec4 &o) const noexcept
    {
        Vec4 r;
        _mm_storeu_ps(&r.x, _mm_add_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&o.x)));
        return r;
    }

    Vec4 operator-(const Vec4 &o) const noexcept
    {
        Vec4 r;
        _mm_storeu_ps(&r.x, _mm_sub_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&o.x)));
        return r;
    }

    Vec4 operator*(float s) const noexcept
    {
        Vec4 r;
        _mm_storeu_ps(&r.x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_set1_ps(s)));
        return r;
    }

    Vec4 &operator+=(const Vec4 &o) noexcept
    {
        _mm_storeu_ps(&x, _mm_add_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&o.x)));
        return *this;
    }

    Vec4 &operator*=(float s) noexcept
    {
        _mm_storeu_ps(&x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_set1_ps(s)));
        return *this;
    }
};

namespace vec4
{
// xyz only: w = 0
// result = a.yzx * b.zxy - a.zxy * b.yzx
inline Vec4 cross(const Vec4 &a, const Vec4 &b) noexcept
{
    __m128 va = _mm_loadu_ps(&a.x);
    __m128 vb = _mm_loadu_ps(&b.x);
    __m128 a_yzx = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 b_zxy = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 a_zxy = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 b_yzx = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 0, 2, 1));
    Vec4 r;
    _mm_storeu_ps(&r.x, _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx)));
    return r;
}

// all 4 components
inline float dot(const Vec4 &a, const Vec4 &b) noexcept
{
    __m128 t = _mm_mul_ps(_mm_loadu_ps(&a.x), _mm_loadu_ps(&b.x));
    t = _mm_add_ps(t, _mm_movehl_ps(t, t));     // [x+z, y+w, ...]
    t = _mm_add_ss(t, _mm_shuffle_ps(t, t, 1)); // [x+y+z+w, ...]
    return _mm_cvtss_f32(t);
}

// xyz only: ignores w
inline float length(const Vec4 &v) noexcept
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vec4 normalize(const Vec4 &v) noexcept
{
    float len = length(v);
    if (len == 0.0f)
    {
        return v;
    }
    return v * (1.0f / len);
}
} // namespace vec4

// column-major + broadcast scalars
struct alignas(16) Mat4
{
    __m128 col[4];

    Mat4() noexcept = default;

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

    [[nodiscard]] Vec4 operator*(const Vec4 &v) const noexcept
    {
        __m128 vec = _mm_loadu_ps(&v.x);

        __m128 vx = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 vy = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 vz = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));
        __m128 vw = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3));

        __m128 r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col[0], vx), _mm_mul_ps(col[1], vy)),
                              _mm_add_ps(_mm_mul_ps(col[2], vz), _mm_mul_ps(col[3], vw)));

        Vec4 out;
        _mm_storeu_ps(&out.x, r);
        return out;
    }
};

namespace mat4
{
inline const Mat4 IDENTITY = []() noexcept {
    Mat4 m;
    m.col[0] = _mm_set_ps(0, 0, 0, 1);
    m.col[1] = _mm_set_ps(0, 0, 1, 0);
    m.col[2] = _mm_set_ps(0, 1, 0, 0);
    m.col[3] = _mm_set_ps(1, 0, 0, 0);
    return m;
}();

// Gauss-Jordan inverse
// column-major: [row r, col c] = data[c*4+r].
inline Mat4 inverse(const Mat4 &m) noexcept
{
    float a[16];
    memcpy(a, m.data(), sizeof(a));
    Mat4 result = IDENTITY;
    float *b = result.data();

    for (int p = 0; p < 4; p++)
    {
        // partial pivot: find max in column p at rows p..3
        int pivot = p;
        float maxVal = a[p * 4 + p] < 0 ? -a[p * 4 + p] : a[p * 4 + p];
        for (int r = p + 1; r < 4; r++)
        {
            float v = a[p * 4 + r] < 0 ? -a[p * 4 + r] : a[p * 4 + r];
            if (v > maxVal)
            {
                maxVal = v;
                pivot = r;
            }
        }
        if (maxVal < 1e-6f)
        {
            return IDENTITY; // singular
        }
        // swap rows p and pivot in both a and b
        if (pivot != p)
        {
            for (int c = 0; c < 4; c++)
            {
                float tmp = a[c * 4 + p];
                a[c * 4 + p] = a[c * 4 + pivot];
                a[c * 4 + pivot] = tmp;
                tmp = b[c * 4 + p];
                b[c * 4 + p] = b[c * 4 + pivot];
                b[c * 4 + pivot] = tmp;
            }
        }
        // scale pivot row
        float inv = 1.0f / a[p * 4 + p];
        for (int c = 0; c < 4; c++)
        {
            a[c * 4 + p] *= inv;
            b[c * 4 + p] *= inv;
        }
        // eliminate column p from all other rows
        for (int r = 0; r < 4; r++)
        {
            if (r == p)
            {
                continue;
            }
            float factor = a[p * 4 + r];
            for (int c = 0; c < 4; c++)
            {
                a[c * 4 + r] -= factor * a[c * 4 + p];
                b[c * 4 + r] -= factor * b[c * 4 + p];
            }
        }
    }

    return result;
}
} // namespace mat4

#endif // MAT4_H
