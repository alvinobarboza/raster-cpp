#pragma once
#include <algorithm>
#include <ranges>
#include <span>
#include <vector>

#include "engine/screen_tile.h"
#include "transforms/vec2.h"

class ShadowMap {
    std::vector<float> shadow_map{};
public:
    static constexpr int TILE_SIZE = 32;
    Grid grid {};
    const int width {}, height {};
    const int half_width {}, half_height {};

    explicit ShadowMap(int width = 1024, int height = 1024) noexcept;

    void clear() noexcept;
    void depth_test(int index, float z) noexcept;
    [[nodiscard]] float sample(Vec2 uv) const noexcept;

    void update_tiles() noexcept;
    void bin_triangles(std::span<ShadowTriangle> triangles) noexcept;
};

inline ShadowMap::ShadowMap(const int width, const int height) noexcept
: width(width), height(height), half_width(width/2), half_height(height/2)
{
    shadow_map.resize(width*height);
    update_tiles();
    clear();
}

inline void ShadowMap::clear() noexcept
{
    std::ranges::fill(shadow_map, 1.0f);
    std::ranges::for_each(grid.tiles, [](Tile& tile)
    {
        tile.triangles_id.clear();
    });
}

inline void ShadowMap::depth_test(const int index, const float z) noexcept
{
    if ( index < width*height && shadow_map[index] > z )
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

inline void ShadowMap::update_tiles() noexcept
{
    const int tile_x = (width + TILE_SIZE - 1) / TILE_SIZE ;
    const int tile_y = (height + TILE_SIZE - 1) / TILE_SIZE ;
    const int total_tiles = tile_x * tile_y;

    grid.width = tile_x;
    grid.height = tile_y;
    grid.total_tiles = total_tiles;
    grid.tiles.clear();
    grid.tiles.resize(total_tiles);

    for (int g_y = 0; g_y < tile_y; ++g_y)
    {
        const int offset_y = g_y * TILE_SIZE;
        for (int g_x = 0; g_x < tile_x; ++g_x)
        {
            const int offset_x = g_x * TILE_SIZE;
            const auto tile_i = g_y * tile_x + g_x;
            grid.tiles[tile_i].offset_x = offset_x;
            grid.tiles[tile_i].offset_y = offset_y;
        }
    }
}

inline void ShadowMap::bin_triangles(std::span<ShadowTriangle> triangles) noexcept
{
    for (const auto [id, tri] : std::views::enumerate(triangles))
    {
        const auto tri_min_y = std::max(static_cast<int>(tri.aabb.min.y), 0);
        const auto tri_max_y = std::min(static_cast<int>(tri.aabb.max.y), height - 1);
        const auto tri_min_x = std::max(static_cast<int>(tri.aabb.min.x), 0);
        const auto tri_max_x = std::min(static_cast<int>(tri.aabb.max.x), width - 1);

        const int grid_min_x {tri_min_x / TILE_SIZE}, grid_max_x {tri_max_x / TILE_SIZE};
        const int grid_min_y {tri_min_y / TILE_SIZE}, grid_max_y {tri_max_y / TILE_SIZE};

        if (grid_min_x == grid_max_x && grid_min_y == grid_max_y)
        {
            const auto index = grid_min_x + grid.width * grid_max_y;
            grid.tiles[index].triangles_id.push_back(static_cast<int>(id));
            continue;
        }

        const auto delta_w0_x = (tri.screen_points[1].y - tri.screen_points[2].y) * TILE_SIZE;
        const auto delta_w1_x = (tri.screen_points[2].y - tri.screen_points[0].y) * TILE_SIZE;
        const auto delta_w2_x = (tri.screen_points[0].y - tri.screen_points[1].y) * TILE_SIZE;

        const auto delta_w0_y = (tri.screen_points[2].x - tri.screen_points[1].x) * TILE_SIZE;
        const auto delta_w1_y = (tri.screen_points[0].x - tri.screen_points[2].x) * TILE_SIZE;
        const auto delta_w2_y = (tri.screen_points[1].x - tri.screen_points[0].x) * TILE_SIZE;

        const auto min_x = static_cast<float>(grid_min_x * TILE_SIZE);
        const auto min_y = static_cast<float>(grid_min_y * TILE_SIZE);

        const Vec3 top_left{min_x, min_y, 0.0f};
        auto w0_y = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], top_left);
        auto w1_y = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], top_left);
        auto w2_y = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], top_left);

        for (int y = grid_min_y; y <= grid_max_y; ++y)
        {
            auto w0 = w0_y;
            auto w1 = w1_y;
            auto w2 = w2_y;
            for (int x = grid_min_x; x <= grid_max_x; ++x)
            {
                const auto tl_w0 = w0;
                const auto tl_w1 = w1;
                const auto tl_w2 = w2;

                const auto tr_w0 = tl_w0 + delta_w0_x;
                const auto tr_w1 = tl_w1 + delta_w1_x;
                const auto tr_w2 = tl_w2 + delta_w2_x;

                const auto bl_w0 = tl_w0 + delta_w0_y;
                const auto bl_w1 = tl_w1 + delta_w1_y;
                const auto bl_w2 = tl_w2 + delta_w2_y;

                const auto br_w0 = bl_w0 + delta_w0_x;
                const auto br_w1 = bl_w1 + delta_w1_x;
                const auto br_w2 = bl_w2 + delta_w2_x;

                if (tl_w0 < 0.0f && tr_w0 < 0.0f && bl_w0 < 0.0f && br_w0 < 0.0f)
                {
                    w0 += delta_w0_x;
                    w1 += delta_w1_x;
                    w2 += delta_w2_x;
                    continue;
                };
                if (tl_w1 < 0.0f && tr_w1 < 0.0f && bl_w1 < 0.0f && br_w1 < 0.0f)
                {
                    w0 += delta_w0_x;
                    w1 += delta_w1_x;
                    w2 += delta_w2_x;
                    continue;
                };
                if (tl_w2 < 0.0f && tr_w2 < 0.0f && bl_w2 < 0.0f && br_w2 < 0.0f)
                {
                    w0 += delta_w0_x;
                    w1 += delta_w1_x;
                    w2 += delta_w2_x;
                    continue;
                };

                const auto index = x + grid.width * y;
                grid.tiles[index].triangles_id.push_back(static_cast<int>(id));

                w0 += delta_w0_x;
                w1 += delta_w1_x;
                w2 += delta_w2_x;
            }
            w0_y += delta_w0_y;
            w1_y += delta_w1_y;
            w2_y += delta_w2_y;
        }
    }
}
