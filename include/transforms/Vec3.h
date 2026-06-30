#pragma once

class Vec3 {
public:
    float x, y, z;

    Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vec3() : x(0), y(0), z(0) {}

    ~Vec3() = default;

    // Get used to dot product, as there is no real operation involving multiplying two vec3
    // and getting a vec3 back
    float operator*(const Vec3 &other) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;

    [[nodiscard]] float length() const;
    [[nodiscard]] Vec3 normalized() const;
    [[nodiscard]] Vec3 cross(const Vec3 &other) const;
    [[nodiscard]] Vec3 lerp_to(const Vec3 &other, float t) const;

};
