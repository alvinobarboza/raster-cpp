#pragma once
#include <cmath>
#include <ostream>

#include "constants.h"
#include "matrix4x4.h"

class Matrix4x4;

class Vec3 {
public:
    float x {};
    float y {};
    float z {};

    explicit Vec3(const float v): x(v), y(v), z(v) {}
    Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vec3() = default;

    ~Vec3() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec3 &rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    Vec3 operator*(const float scalar) const
    {
        return {
            x * scalar,
            y * scalar,
            z * scalar
        };
    }

    Vec3 operator/(const float scalar) const
    {
        if (scalar == 0.0f) return {0.0f, 0.0f, 0.0f};
        return {
            x / scalar,
            y / scalar,
            z / scalar
        };
    }

    Vec3 operator/(const Vec3 &rhs) const
    {
        return {
            x / rhs.x,
            y / rhs.y,
            z / rhs.z
        };
    }

    Vec3& operator+=(const Vec3 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vec3 operator+(Vec3 rhs) const
    {
        rhs += *this;
        return rhs;
    }

    Vec3& operator-=(const Vec3 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vec3 operator-(const Vec3 &rhs) const
    {
        return {
            x - rhs.x,
            y - rhs.y,
            z - rhs.z
        };
    }

    Vec3 operator-() const
    {
        return {-x,-y,-z};
    }

    Vec3 operator*(const Matrix4x4 &rhs) const
    {
        return {
            (x * rhs(0,0)) + (y * rhs(1,0)) + (z * rhs(2,0)) + (1.0f * rhs(3,0)),
            (x * rhs(0,1)) + (y * rhs(1,1)) + (z * rhs(2,1)) + (1.0f * rhs(3,1)),
            (x * rhs(0,2)) + (y * rhs(1,2)) + (z * rhs(2,2)) + (1.0f * rhs(3,2)),
        };
    }

    [[nodiscard]] float length() const
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    [[nodiscard]] Vec3 normalized() const
    {
        return *this / length();
    }

    [[nodiscard]] Vec3 cross(const Vec3 &rhs) const
    {
        return {
            y*rhs.z - z*rhs.y,
            z*rhs.x - x*rhs.z,
            x*rhs.y - y*rhs.x,
        };
    }

    [[nodiscard]] Vec3 interpolate(const Vec3 &rhs, const float t) const
    {
        return {
            transforms::linear_interpolation(this->x, rhs.x, t),
            transforms::linear_interpolation(this->y, rhs.y, t),
            transforms::linear_interpolation(this->z, rhs.z, t),
        };
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec3 &v)
    {
        os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z << ")";
        return os;
    }
};
