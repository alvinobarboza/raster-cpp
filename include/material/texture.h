#pragma once
#include <vector>

#include "transforms/vec2.h"
#include "transforms/vec4.h"

// must be powers of 2 128, 256...
class TextureRaster {
public:
    std::vector<Vec4> buffer = {};
    int width = 0, height = 0;
    int width_mask = 0, height_mask = 0;
    float f_width = 0, f_height = 0;

    [[nodiscard]] Vec4 texel_color(const Vec2 &uv) const;
    [[nodiscard]] float texel_intensity(const Vec2 &uv) const;
};
