#include "transforms/Vec3.h"
#include "transforms/Matrix4x4.h"
wa
#include <cmath>

float Vec3::operator*(const Vec3 &rhs) const
{
    return x*rhs.x + y*rhs.y + z*rhs.z;
}

Vec3 Vec3::operator*(const float scalar) const
{
    return {
        x * scalar,
        y * scalar,
        z * scalar
    };
}

Vec3 Vec3::operator*(const Matrix4x4 &rhs) const {
    return {
        (x * rhs.at(0,0)) + (y * rhs.at(1,0)) + (z * rhs.at(2,0)) + (1 * rhs.at(3,0)),
        (x * rhs.at(0,1)) + (y * rhs.at(1,1)) + (z * rhs.at(2,1)) + (1 * rhs.at(3,1)),
        (x * rhs.at(0,2)) + (y * rhs.at(1,2)) + (z * rhs.at(2,2)) + (1 * rhs.at(3,2)),
    };
}

Vec3 Vec3::operator/(const float scalar) const
{
    if (scalar == 0.0f) return {0.0f, 0.0f, 0.0f};
    return {
        x / scalar,
        y / scalar,
        z / scalar
    };
}

Vec3 &Vec3::operator+=(const Vec3 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

Vec3 Vec3::operator+(Vec3 rhs) const
{
    rhs += *this;
    return rhs;
}

Vec3 &Vec3::operator-=(const Vec3 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

Vec3 Vec3::operator-(const Vec3 rhs) const
{
    return {
        x - rhs.x,
        y - rhs.y,
        z - rhs.z
    };
}

Vec3 Vec3::operator-() const
{
    return {-x,-y,-z};
}

float Vec3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

Vec3 Vec3::normalized() const {
    return *this / length();
}

Vec3 Vec3::cross(const Vec3 &rhs) const {
    return {
        y*rhs.z - z*rhs.y,
        z*rhs.x - x*rhs.z,
        x*rhs.y - y*rhs.x,
    };
}

Vec3 Vec3::lerp_to(const Vec3 &rhs, const float t) const {
    if (t <= 0.0f) {
        return *this;
    }
    if (t >= 1.0f) {
        return rhs;
    }
    return *this + (rhs - *this) * t;
}

std::ostream& operator<<(std::ostream &os, const Vec3 &v)
{
    os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z << ")";
    return os;
}