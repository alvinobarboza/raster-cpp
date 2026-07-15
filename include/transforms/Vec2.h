#pragma once
#include <iostream>

class Vec2 {
public:
    float x = 0.0f;
    float y = 0.0f;

    Vec2(const float x, const float y) : x(x), y(y) {}
    Vec2() = default;

    ~Vec2() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec2 &rhs) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;
    Vec2& operator+=(const Vec2 &rhs);
    Vec2 operator+(Vec2 rhs) const;
    Vec2& operator-=(const Vec2 &rhs);
    Vec2 operator-(const Vec2 &rhs) const;
    Vec2 operator-() const;

    [[nodiscard]] float length() const;
    [[nodiscard]] Vec2 normalized() const;
    [[nodiscard]] Vec2 lerp_to(const Vec2 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const Vec2 &v);
};
