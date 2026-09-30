#pragma once
#include <algorithm>
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
    [[nodiscard]] float sample(Vec2 uv) const noexcept;
};

inline ShadowMap::ShadowMap(const int width, const int height) noexcept
: width(width), height(height), half_width(width/2), half_height(height/2)
{
    shadow_map.resize(width*height);
    clear();
}

inline void ShadowMap::clear() noexcept
{
    std::ranges::fill(shadow_map, 1.0f);
}

inline void ShadowMap::depth_test(const int x, const int y, const float z) noexcept
{
    if (const auto index {x + y * width}; index < width*height && shadow_map[index] > z)
    {
        shadow_map[index] = z;
    }
}

inline float ShadowMap::sample(const Vec2 uv) const noexcept
{
    const int x = static_cast<int>(uv.x * static_cast<float>(width));
    const int y = static_cast<int>(uv.y * static_cast<float>(height));

    if (x < 0 || x >= width || y < 0 || y >= height) return 1e5f;

    return shadow_map[x + y * width];
}