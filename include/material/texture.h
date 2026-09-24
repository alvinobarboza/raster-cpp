#pragma once
#include <vector>

#include "transforms/vec2.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

// must be powers of 2 128, 256...
class TextureRaster {
public:
    std::vector<Vec4> buffer {};
    std::vector<float> buffer_float {};
    int width {}, height {};
    int width_mask {}, height_mask {};
    float f_width {}, f_height {};

    Vec2 texel_coord(Vec2 uv) const noexcept;

    [[nodiscard]] Vec4 texel_color(Vec2 uv) const noexcept;
    [[nodiscard]] Vec3 texel_normal(Vec2 uv) const noexcept;
    [[nodiscard]] float texel_intensity(Vec2 uv) const noexcept;

    [[nodiscard]] Vec4 bilinear_color(Vec2 uv) const noexcept;
};
