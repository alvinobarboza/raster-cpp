#pragma once
#include <ostream>

class matrix4x4;

class vec4 {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    vec4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
    vec4() = default;

    ~vec4() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const vec4 &rhs) const;
    vec4 operator*(float scalar) const;
    vec4 operator/(float scalar) const;
    vec4& operator+=(const vec4 &rhs);
    vec4 operator+(vec4 rhs) const;
    vec4& operator-=(const vec4 &rhs);
    vec4 operator-(const vec4 &rhs) const;
    vec4 operator-() const;

    vec4 operator*(const matrix4x4 &rhs) const;

    [[nodiscard]] float length() const;
    [[nodiscard]] vec4 normalized() const;
    [[nodiscard]] vec4 lerp_to(const vec4 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const vec4 &v);
};
