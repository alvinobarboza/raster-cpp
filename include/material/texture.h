#pragma once
#include <array>
#include <cmath>
#include <vector>

#include "../../cmake-build-debug/_deps/raylib-src/src/raylib.h"
#include "transforms/vec2.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

struct BilinearIndices {
    int tl{}, tr{}, bl{}, br{};
    float wx{}, wy{};
};

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

    [[nodiscard]] BilinearIndices bilinear_coord(Vec2 uv) const noexcept;

public:
    std::vector<Color> buffer {};
    std::vector<unsigned char> buffer_value {};
    int width {}, height {};
    int width_mask {}, height_mask {};
    float f_width {}, f_height {};

    [[nodiscard]] Vec3 texel_normal(Vec2 uv) const noexcept;
    [[nodiscard]] float texel_intensity(Vec2 uv) const noexcept;

    [[nodiscard]] Vec4 bilinear_color(Vec2 uv) const noexcept;
    [[nodiscard]] Vec4 bilinear_color_gamma(Vec2 uv) const noexcept;
};

inline BilinearIndices TextureRaster::bilinear_coord(const Vec2 uv) const noexcept
{
    const float inverted_y = 1.0f - uv.y;
    const float xf {uv.x * f_width};
    const float yf {inverted_y * f_height};

    const int ix {static_cast<int>(xf)};
    const int iy {static_cast<int>(yf)};

    const float wx {xf - static_cast<float>(ix)};
    const float wy {yf - static_cast<float>(iy)};

    const int x0 {ix & width_mask};
    const int x1 {(ix + 1) & width_mask};

    const int y0 {iy & height_mask};
    const int y1 {(iy + 1) & height_mask};

    const int row0 {y0 * width};
    const int row1 {y1 * width};

    const int idx_tl {row0 + x0};
    const int idx_tr {row0 + x1};
    const int idx_bl {row1 + x0};
    const int idx_br {row1 + x1};

    return {
        idx_tl, idx_tr, idx_bl, idx_br, wx, wy,
    };
}