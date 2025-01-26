
#ifndef AP_COMMON_UTIL_H
#define AP_COMMON_UTIL_H

#include <cmath>

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

struct Vec2
{
    float x = 0;
    float y = 0;
};

inline Vec2 operator*(const Vec2& v, float s)
{
    return { v.x * s, v.y * s };
}

inline Vec2 operator/(const Vec2& v, float s)
{
    return { v.x / s, v.y / s };
}

#endif // AP_COMMON_UTIL_H
