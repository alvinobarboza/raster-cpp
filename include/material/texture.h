#pragma once
#include <vector>

#include "transforms/Vec2.h"
#include "transforms/Vec4.h"

class Texture {
public:
    std::vector<Vec4> buffer = {};
    int width = 0, height = 0;
    int width_mask = 0, height_mask = 0;
    float f_width = 0, f_height = 0;

    [[nodiscard]] Vec4 texel_color(const Vec2 &uv) const;
    [[nodiscard]] float texel_intensity(const Vec2 &uv) const;
};
