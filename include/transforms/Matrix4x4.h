#pragma once
#include <array>
#include <ostream>

class Vec3;

class Matrix4x4 {
    std::array<float, 4*4> data {};
    static constexpr size_t length = 4;

public:
    Matrix4x4() = default;
    Matrix4x4(
        const float m00, const float m01, const float m02, const float m03,
        const float m10, const float m11, const float m12, const float m13,
        const float m20, const float m21, const float m22, const float m23,
        const float m30, const float m31, const float m32, const float m33)
        : data{
            m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33
          }
    {}

    void to_identity();
    void to_scale(const Vec3& scale);
    void to_rotation(const Vec3& angle);
    void to_translation(const Vec3& translation);

    [[nodiscard]] Matrix4x4 transpose() const;
    [[nodiscard]] Matrix4x4 operator*( const Matrix4x4& rhs) const;
    [[nodiscard]] float operator()(size_t x, size_t y) const;

    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m);
};
