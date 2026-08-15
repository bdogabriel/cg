#ifndef VEC4_H
#define VEC4_H

#include <cmath>
#include <xmmintrin.h>

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

#endif // VEC4_H