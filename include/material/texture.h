#pragma once
#include <vector>

#include "transforms/vec2.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

// must be powers of 2 128, 256...
class TextureRaster {
public:
    std::vector<Vec4> buffer {};
    int width {}, height {};
    int width_mask {}, height_mask {};
    float f_width {}, f_height {};

    [[nodiscard]] Vec4 texel_color(const Vec2 &uv) const;
    [[nodiscard]] Vec3 texel_normal(const Vec2 &uv) const;
    [[nodiscard]] float texel_intensity(const Vec2 &uv) const;
};
