#pragma once
#include <ostream>

class Matrix4x4;

class Vec3 {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vec3() = default;

    ~Vec3() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec3 &rhs) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;
    Vec3& operator+=(const Vec3 &rhs);
    Vec3 operator+(Vec3 rhs) const;
    Vec3& operator-=(const Vec3 &rhs);
    Vec3 operator-(const Vec3 &rhs) const;
    Vec3 operator-() const;

    Vec3 operator*(const Matrix4x4 &rhs) const;

    [[nodiscard]] float length() const;
    [[nodiscard]] Vec3 normalized() const;
    [[nodiscard]] Vec3 cross(const Vec3 &rhs) const;
    [[nodiscard]] Vec3 lerp_to(const Vec3 &rhs, float t) const;

    friend std::ostream &operator<<(std::ostream &os, const Vec3 &v);
};
