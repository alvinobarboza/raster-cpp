#include "transforms/vec3.h"
#include "transforms/matrix4x4.h"

#include <cmath>

float vec3::operator*(const vec3 &rhs) const
{
    return x*rhs.x + y*rhs.y + z*rhs.z;
}

vec3 vec3::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar,
        z * scalar
    };
}

vec3 vec3::operator*(const matrix4x4 &rhs) const
{
    return {
        (x * rhs(0,0)) + (y * rhs(1,0)) + (z * rhs(2,0)) + (1.0f * rhs(3,0)),
        (x * rhs(0,1)) + (y * rhs(1,1)) + (z * rhs(2,1)) + (1.0f * rhs(3,1)),
        (x * rhs(0,2)) + (y * rhs(1,2)) + (z * rhs(2,2)) + (1.0f * rhs(3,2)),
    };
}

vec3 vec3::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {0.0f, 0.0f, 0.0f};
    return {
        x / scalar,
        y / scalar,
        z / scalar
    };
}

vec3 &vec3::operator+=(const vec3 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

vec3 vec3::operator+(vec3 rhs) const
{
    rhs += *this;
    return rhs;
}

vec3 &vec3::operator-=(const vec3 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

vec3 vec3::operator-(const vec3 &rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
        z - rhs.z
    };
}

vec3 vec3::operator-() const
{
    return {-x,-y,-z};
}

float vec3::length() const
{
    return std::sqrt(x*x + y*y + z*z);
}

vec3 vec3::normalized() const
{
    return *this / length();
}

vec3 vec3::cross(const vec3 &rhs) const
{
    return {
        y*rhs.z - z*rhs.y,
        z*rhs.x - x*rhs.z,
        x*rhs.y - y*rhs.x,
    };
}

vec3 vec3::lerp_to(const vec3 &rhs, const float t) const
{
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const vec3 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z << ")";
    return os;
}