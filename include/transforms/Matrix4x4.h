#pragma once
#include <array>
#include <ostream>

class Vec3;

class Matrix4x4 {
    std::array<float, 4*4> data = std::array<float, 4*4>{
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    };
    const unsigned int length = 4;

public:
    Matrix4x4() = default;
    Matrix4x4(
        const float m00, const float m01, const float m02, const float m03,
        const float m10, const float m11, const float m12, const float m13,
        const float m20, const float m21, const float m22, const float m23,
        const float m30, const float m31, const float m32, const float m33)
    {
        data[0] = m00; data[1] = m01; data[2] = m02; data[3] = m03;
        data[4] = m10; data[5] = m11; data[6] = m12; data[7] = m13;
        data[8] = m20; data[9] = m21; data[10] = m22; data[11] = m23;
        data[12] = m30; data[13] = m31; data[14] = m32; data[15] = m33;
    }

    void to_identity();
    void to_scale(const Vec3& scale);
    void to_rotation(const Vec3& angle);
    void to_translation(const Vec3& translation);

    [[nodiscard]] float at(unsigned int x, unsigned int y) const;
    [[nodiscard]] Matrix4x4 transpose() const;

    Matrix4x4 operator*( const Matrix4x4& rhs) const;
    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m);
};
