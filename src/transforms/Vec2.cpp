#include "transforms/Vec2.h"

#include <cmath>

float Vec2::operator*(const Vec2 &rhs) const
{
    return x*rhs.x + y*rhs.y;
}

Vec2 Vec2::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar
    };
}

Vec2 Vec2::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {0.0f, 0.0f};
    return {
        x / scalar,
        y / scalar
    };
}

Vec2 &Vec2::operator+=(const Vec2 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vec2 Vec2::operator+(Vec2 rhs) const
{
    rhs += *this;
    return rhs;
}

Vec2 &Vec2::operator-=(const Vec2 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vec2 Vec2::operator-(const Vec2 &rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
    };
}

Vec2 Vec2::operator-() const
{
    return {-x,-y};
}

float Vec2::length() const {
    return std::sqrt(x*x + y*y);
}

Vec2 Vec2::normalized() const {
    return *this / length();
}

Vec2 Vec2::lerp_to(const Vec2 &rhs, const float t) const {
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const Vec2 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ")";
    return os;
}