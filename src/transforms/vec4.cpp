#include "transforms/vec4.h"
#include "transforms/matrix4x4.h"

#include <cmath>

float vec4::operator*(const vec4 &rhs) const
{
    return x*rhs.x + y*rhs.y + z*rhs.z;
}

vec4 vec4::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar,
        z * scalar,
        w * scalar
    };
}

vec4 vec4::operator*(const matrix4x4& rhs) const {
    return {
        rhs(0,0)* x + rhs(1,0)*y + rhs(2,0)*z + rhs(3,0)*w,
        rhs(0,1)*x + rhs(1,1)*y + rhs(2,1)*z + rhs(3,1)*w,
        rhs(0,2)*x + rhs(1,2)*y + rhs(2,2)*z + rhs(3,2)*w,
        rhs(0,3)*x + rhs(1,3)*y + rhs(2,3)*z + rhs(3,3)*w
    };
}

vec4 vec4::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {};
    return {
        x / scalar,
        y / scalar,
        z / scalar,
        w / scalar
    };
}

vec4 &vec4::operator+=(const vec4 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

vec4 vec4::operator+(vec4 rhs) const
{
    rhs += *this;
    return rhs;
}

vec4 &vec4::operator-=(const vec4 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

vec4 vec4::operator-(const vec4 &rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
        z - rhs.z,
        w - rhs.w
    };
}

vec4 vec4::operator-() const
{
    return {-x,-y,-z,-w};
}

float vec4::length() const
{
    return std::sqrt(x*x + y*y + z*z + w*w);
}

vec4 vec4::normalized() const
{
    return *this / length();
}

vec4 vec4::lerp_to(const vec4 &rhs, const float t) const
{
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const vec4 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z <<  ", w:" << v.w << ")";
    return os;
}