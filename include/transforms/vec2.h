#pragma once
#include <cmath>
#include <iostream>

#include "constants.h"

class Vec2 {
public:
    float x {};
    float y {};

    Vec2(const float x, const float y) : x(x), y(y) {}
    Vec2() = default;

    ~Vec2() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec2 rhs) const
    {
        return x*rhs.x + y*rhs.y;
    }

    Vec2 operator*(const float scalar) const
    {
        return {
            x * scalar,
            y * scalar
        };
    }

    Vec2 operator/(const float scalar) const
    {
        if (scalar == 0.0f) return {0.0f, 0.0f};
        return {
            x / scalar,
            y / scalar
        };
    }

    Vec2& operator+=(const Vec2 rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2 operator+(Vec2 rhs) const
    {
        rhs += *this;
        return rhs;
    }

    Vec2& operator-=(const Vec2 rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec2 operator-(const Vec2 rhs) const
    {
        return {
            x - rhs.x,
            y - rhs.y,
        };
    }

    Vec2 operator-() const
    {
        return {-x,-y};
    }

    [[nodiscard]] float length() const
    {
        return std::sqrt(x*x + y*y);
    }

    [[nodiscard]] Vec2 normalized() const
    {
        return *this / length();
    }

    [[nodiscard]] Vec2 interpolate(const Vec2 rhs, const float t) const
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
        };
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec2 v)
    {
        os << "(x:" << v.x << ", y:" << v.y << ")";
        return os;
    }
};
