#include "lights/shadow.h"

#include <algorithm>

ShadowMap::ShadowMap(const int width, const int height) noexcept
: width(width), height(height), half_width(width/2), half_height(height/2)
{
    shadow_map.resize(width*height);
    clear();
}

void ShadowMap::clear() noexcept
{
    std::ranges::fill(shadow_map, 1e5f);
}

void ShadowMap::depth_test(const int x, const int y, const float z) noexcept
{
    if (const auto index {x + y * width}; index < width*height && shadow_map[index] > z)
    {
        shadow_map[index] = z;
    }
}

float ShadowMap::sample(const Vec2 uv) const noexcept
{
    const int x = static_cast<int>(uv.x * static_cast<float>(width));
    const int y = static_cast<int>(uv.y * static_cast<float>(height));

    if (x < 0 || x >= width || y < 0 || y >= height) return 1e5f;

    return shadow_map[x + y * width];
}
