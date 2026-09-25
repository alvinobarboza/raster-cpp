#pragma once
#include <vector>

#include "transforms/vec2.h"

class ShadowMap {
    std::vector<float> shadow_map{};
public:
    const int width {}, height {};
    const int half_width {}, half_height {};

    explicit ShadowMap(int width = 1024, int height = 1024) noexcept;
    void clear() noexcept;
    void depth_test(int x, int y, float z) noexcept;
    float sample(Vec2 uv) const noexcept;
};
