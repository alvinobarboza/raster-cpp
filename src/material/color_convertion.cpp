#include "material/color_convertion.h"

Color color_convertion::vec4_to_color(const Vec4 &vec)
{
    return {
        static_cast<unsigned char>(255.0f/vec.x),
        static_cast<unsigned char>(255.0f/vec.y),
        static_cast<unsigned char>(255.0f/vec.z),
        static_cast<unsigned char>(255.0f/vec.w)
        };
}
Vec4 color_convertion::color_to_vec4(const Color &color) {
    return {
        static_cast<float>(color.r)/255.0f,
        static_cast<float>(color.g)/255.0f,
        static_cast<float>(color.b)/255.0f,
        static_cast<float>(color.a)/255.0f
        };
}