#pragma once
#include <ostream>

class matrix4x4;

class vec3 {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    vec3() = default;

    ~vec3() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const vec3 &rhs) const;
    vec3 operator*(float scalar) const;
    vec3 operator/(float scalar) const;
    vec3& operator+=(const vec3 &rhs);
    vec3 operator+(vec3 rhs) const;
    vec3& operator-=(const vec3 &rhs);
    vec3 operator-(const vec3 &rhs) const;
    vec3 operator-() const;

    vec3 operator*(const matrix4x4 &rhs) const;

    [[nodiscard]] float length() const;
    [[nodiscard]] vec3 normalized() const;
    [[nodiscard]] vec3 cross(const vec3 &rhs) const;
    [[nodiscard]] vec3 lerp_to(const vec3 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const vec3 &v);
};
