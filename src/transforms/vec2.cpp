#include "transforms/vec2.h"

#include <cmath>

float vec2::operator*(const vec2 &rhs) const
{
    return x*rhs.x + y*rhs.y;
}

vec2 vec2::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar
    };
}

vec2 vec2::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {0.0f, 0.0f};
    return {
        x / scalar,
        y / scalar
    };
}

vec2 &vec2::operator+=(const vec2 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

vec2 vec2::operator+(vec2 rhs) const
{
    rhs += *this;
    return rhs;
}

vec2 &vec2::operator-=(const vec2 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

vec2 vec2::operator-(const vec2 &rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
    };
}

vec2 vec2::operator-() const
{
    return {-x,-y};
}

float vec2::length() const {
    return std::sqrt(x*x + y*y);
}

vec2 vec2::normalized() const {
    return *this / length();
}

vec2 vec2::lerp_to(const vec2 &rhs, const float t) const {
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const vec2 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ")";
    return os;
}