#include "transforms/Vec4.h"
#include "transforms/Matrix4x4.h"

#include <cmath>

float Vec4::operator*(const Vec4 &rhs) const
{
    return x*rhs.x + y*rhs.y + z*rhs.z;
}

Vec4 Vec4::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar,
        z * scalar,
        w * scalar
    };
}

Vec4 Vec4::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {};
    return {
        x / scalar,
        y / scalar,
        z / scalar,
        w / scalar
    };
}

Vec4 &Vec4::operator+=(const Vec4 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

Vec4 Vec4::operator+(Vec4 rhs) const
{
    rhs += *this;
    return rhs;
}

Vec4 &Vec4::operator-=(const Vec4 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

Vec4 Vec4::operator-(const Vec4 &rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
        z - rhs.z,
        w - rhs.w
    };
}

Vec4 Vec4::operator-() const
{
    return {-x,-y,-z,-w};
}

float Vec4::length() const
{
    return std::sqrt(x*x + y*y + z*z + w*w);
}

Vec4 Vec4::normalized() const
{
    return *this / length();
}

Vec4 Vec4::lerp_to(const Vec4 &rhs, const float t) const
{
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const Vec4 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z <<  ", w:" << v.w << ")";
    return os;
}