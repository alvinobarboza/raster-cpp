#pragma once
#include <ostream>

class Matrix4x4;

class Vec4 {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
    Vec4() = default;

    ~Vec4() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec4 &rhs) const;
    Vec4 operator*(float scalar) const;
    Vec4 operator/(float scalar) const;
    Vec4& operator+=(const Vec4 &rhs);
    Vec4 operator+(Vec4 rhs) const;
    Vec4& operator-=(const Vec4 &rhs);
    Vec4 operator-(const Vec4 &rhs) const;
    Vec4 operator-() const;

    [[nodiscard]] float length() const;
    [[nodiscard]] Vec4 normalized() const;
    [[nodiscard]] Vec4 lerp_to(const Vec4 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const Vec4 &v);
};
