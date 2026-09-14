#include "material/color_convertion.h"

Color color_convertion::vec4_to_color(const Vec4 &vec)
{
    return {
        static_cast<unsigned char>(vec.x * 255.0f),
        static_cast<unsigned char>(vec.y * 255.0f),
        static_cast<unsigned char>(vec.z * 255.0f),
        static_cast<unsigned char>(vec.w * 255.0f)
    };
}
Vec4 color_convertion::color_to_vec4(const Color &color) {
    constexpr auto reciprocal = 1.0f/255.0f;
    return {
        static_cast<float>(color.r) * reciprocal,
        static_cast<float>(color.g) * reciprocal,
        static_cast<float>(color.b) * reciprocal,
        static_cast<float>(color.a) * reciprocal
    };
}