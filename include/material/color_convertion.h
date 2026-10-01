#pragma once

#include "raylib.h"
#include "transforms/vec4.h"

namespace color_convertion {
    inline Color vec4_to_color(const Vec4 &vec)
    {
        return {
            static_cast<unsigned char>(vec.x * 255.0f),
            static_cast<unsigned char>(vec.y * 255.0f),
            static_cast<unsigned char>(vec.z * 255.0f),
            static_cast<unsigned char>(vec.w * 255.0f)
        };
    }

    inline Vec4 color_to_vec4(const Color color) {
        constexpr auto reciprocal = 1.0f/255.0f;
        return {
            static_cast<float>(color.r) * reciprocal,
            static_cast<float>(color.g) * reciprocal,
            static_cast<float>(color.b) * reciprocal,
            static_cast<float>(color.a) * reciprocal
        };
    }
}
