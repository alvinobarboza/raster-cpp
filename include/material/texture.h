#pragma once
#include <array>
#include <cmath>
#include <vector>

#include "../../cmake-build-debug/_deps/raylib-src/src/raylib.h"
#include "transforms/vec2.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

// must be powers of 2 128, 256...
class TextureRaster {
    // LUT for gamma
    static inline const auto srgb_to_linear = []() {
        std::array<float, 256> table{};
        for (int i = 0; i < 256; ++i) {
            table[i] = std::pow(static_cast<float>(i) / 255.0f, 2.2f);
        }
        return table;
    }();

    // LUT for linear
    static constexpr auto rgb_to_linear = []() {
        std::array<float, 256> table{};
        for (int i = 0; i < 256; ++i) {
            table[i] = static_cast<float>(i) / 255.0f;
        }
        return table;
    }();

    [[nodiscard]] static Vec4 color_to_vec4_gamma(const Color c) noexcept
    {
        return {
            srgb_to_linear[c.r],
            srgb_to_linear[c.g],
            srgb_to_linear[c.b],
            srgb_to_linear[c.a],
        };
    }
    [[nodiscard]] static Vec4 color_to_vec4(const Color c) noexcept
    {
        return {
            rgb_to_linear[c.r],
            rgb_to_linear[c.g],
            rgb_to_linear[c.b],
            rgb_to_linear[c.a],
        };
    }

public:
    std::vector<Color> buffer {};
    std::vector<unsigned char> buffer_value {};
    int width {}, height {};
    int width_mask {}, height_mask {};
    float f_width {}, f_height {};

    [[nodiscard]] Vec2 texel_coord(Vec2 uv) const noexcept;

    [[nodiscard]] Vec3 texel_normal(Vec2 uv) const noexcept;
    [[nodiscard]] float texel_intensity(Vec2 uv) const noexcept;

    [[nodiscard]] Vec4 bilinear_color(Vec2 uv) const noexcept;
    [[nodiscard]] Vec4 bilinear_color_gamma(Vec2 uv) const noexcept;
};
