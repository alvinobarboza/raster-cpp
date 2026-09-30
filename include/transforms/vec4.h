#pragma once
#include <cmath>
#include <ostream>

#include "constants.h"
#include "matrix4x4.h"

class Matrix4x4;

class Vec4 {
public:
    float x {};
    float y {};
    float z {};
    float w {};

    Vec4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
    Vec4() = default;

    ~Vec4() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec4 &rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    Vec4 operator*(const float scalar) const
    {
        return {
            x * scalar,
            y * scalar,
            z * scalar,
            w * scalar
        };
    }

    Vec4 operator/(const float scalar) const
    {
        if (scalar == 0.0f) return {};
        return {
            x / scalar,
            y / scalar,
            z / scalar,
            w / scalar
        };
    }

    Vec4& operator+=(const Vec4 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    Vec4 operator+(Vec4 rhs) const
    {
        rhs += *this;
        return rhs;
    }

    Vec4& operator-=(const Vec4 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    Vec4 operator-(const Vec4 &rhs) const
    {
        return {
            x - rhs.x,
            y - rhs.y,
            z - rhs.z,
            w - rhs.w
        };
    }

    Vec4 operator-() const
    {
        return {-x,-y,-z,-w};
    }

    Vec4 operator*(const Matrix4x4 &rhs) const
    {
        return {
            rhs(0,0)* x + rhs(1,0)*y + rhs(2,0)*z + rhs(3,0)*w,
            rhs(0,1)*x + rhs(1,1)*y + rhs(2,1)*z + rhs(3,1)*w,
            rhs(0,2)*x + rhs(1,2)*y + rhs(2,2)*z + rhs(3,2)*w,
            rhs(0,3)*x + rhs(1,3)*y + rhs(2,3)*z + rhs(3,3)*w
        };
    }


    [[nodiscard]] float length() const
    {
        return std::sqrt(x*x + y*y + z*z + w*w);
    }

    [[nodiscard]] Vec4 normalized() const
    {
        return *this / length();
    }

    [[nodiscard]] Vec4 lerp_to(const Vec4 &rhs, const float t) const
    {
        if (t <= 0.0f) {
            return *this;
        }
        if (t >= 1.0f) {
            return rhs;
        }
        return {
            transforms::lerp(this->x, rhs.x, t),
            transforms::lerp(this->y, rhs.y, t),
            transforms::lerp(this->z, rhs.z, t),
            transforms::lerp(this->w, rhs.w, t),
        };
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec4 &v)
    {
        os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z <<  ", w:" << v.w << ")";
        return os;
    }
};
