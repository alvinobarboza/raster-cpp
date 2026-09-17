#include <ranges>

#include "engine/screen_tile.h"

void ScreenTile::bin_triangles(const std::vector<FullTriangle> &triangles) noexcept
{
    for (const auto [index, triangle] : std::views::enumerate(triangles))
    {
        if (triangle.aabb.collides(aabb))
        {
            is_active = true;
            triangles_id.push_back(index);
        }
    }
}