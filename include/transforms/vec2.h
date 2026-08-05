#pragma once
#include <iostream>

class vec2 {
public:
    float x = 0.0f;
    float y = 0.0f;

    vec2(const float x, const float y) : x(x), y(y) {}
    vec2() = default;

    ~vec2() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const vec2 &rhs) const;
    vec2 operator*(float scalar) const;
    vec2 operator/(float scalar) const;
    vec2& operator+=(const vec2 &rhs);
    vec2 operator+(vec2 rhs) const;
    vec2& operator-=(const vec2 &rhs);
    vec2 operator-(const vec2 &rhs) const;
    vec2 operator-() const;

    [[nodiscard]] float length() const;
    [[nodiscard]] vec2 normalized() const;
    [[nodiscard]] vec2 lerp_to(const vec2 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const vec2 &v);
};
