#include "transforms/Matrix4x4.h"
#include "transforms/Vec3.h"
#include "transforms/Constants.h"

#include <cmath>
#include <iomanip>

void Matrix4x4::to_identity()
{
    data = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

void Matrix4x4::to_scale(const Vec3& scale)
{
    data = {
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f,       1.0f,
    };
}
void Matrix4x4::to_rotation(const Vec3& angle)
{
    const float cosa = std::cos(angle.x * -Transforms::DEG_TO_RAD);
    const float sina = std::sin(angle.x * -Transforms::DEG_TO_RAD);

    const float cosb = std::cos(angle.y * -Transforms::DEG_TO_RAD);
    const float sinb = std::sin(angle.y * -Transforms::DEG_TO_RAD);

    const float cosga = std::cos(angle.z * -Transforms::DEG_TO_RAD);
    const float singa = std::sin(angle.z * -Transforms::DEG_TO_RAD);

    data = {
        cosb * cosga, sina*sinb*cosga - cosa*singa, cosa*sinb*cosga + sina*singa, 0.0f,
        cosb * singa, sina*sinb*singa + cosa*cosga, cosa*sinb*singa - sina*cosga, 0.0f,
        -sinb, sina * cosb, cosa * cosb, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}
void Matrix4x4::to_translation(const Vec3& translation)
{
    data = {
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

float Matrix4x4::operator()(const std::size_t x, const std::size_t y) const
{
    return data[y * length + x];
}

Matrix4x4 Matrix4x4::transpose() const
{
    Matrix4x4 result;

    for (std::size_t y = 0; y < length; y++) {
        for (std::size_t x = 0; x < length; x++) {
            result.data[y*length+x] = data[x*length+y];
        }
    }

    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &rhs) const
{
    Matrix4x4 result;

    for (std::size_t y = 0; y < length; ++y) {
        for (std::size_t x = 0; x < length; ++x) {
            float sum = 0.0f;
            for (std::size_t k = 0; k < length; ++k) {
                sum += (*this)(k, y) * rhs(x, k);
            }
            result.data[x + y * length] = sum;
        }
    }
    return result;
}

std::ostream& operator<<(std::ostream &os, const Matrix4x4 &m)
{
    const std::ios_base::fmtflags f(os.flags());

    os << std::fixed << std::setprecision(3);
    for (std::size_t y = 0; y < 4; ++y) {
        os << "| ";
        for (std::size_t x = 0; x < 4; ++x) {
            os << std::setw(9) << m(x, y) << " ";
        }
        os << "|\n";
    }

    os.flags(f);
    return os;
}