#include <algorithm>
#include <ranges>

#include "engine/viewport.h"

#include "engine/timer.h"
#include "material/color_convertion.h"

Viewport::Viewport(
    const int width, const int height, const int res_factor): res_factor(res_factor)
{
    update_frame_buffer_size(width, height);
}

void Viewport::clear_frame_buffer() noexcept
{
    std::ranges::fill(frame_buffer, BLACK);
    std::ranges::fill(depth_buffer, 1e5f);
}

void Viewport::update_tiles() noexcept
{
    const int tile_x = (width + TILE_SIZE - 1) / TILE_SIZE ;
    const int tile_y = (height + TILE_SIZE - 1) / TILE_SIZE ;
    const int total_tiles = tile_x * tile_y;

    grid.width = tile_x;
    grid.height = tile_y;
    grid.total_tiles = total_tiles;
    grid.tiles.clear();
    grid.tiles.resize(total_tiles);
}

void Viewport::reset_tiles() noexcept
{
    std::ranges::for_each(grid.tiles, [](Tile& tile)
    {
        tile.counter = 0;
        tile.offset = 0;
        tile.cursor_offset = 0;
    });
}

void Viewport::bin_triangles(std::span<FullTriangle> triangles) noexcept
{
    for (const FullTriangle &tri : triangles)
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
            grid.tiles[index].counter++;
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
                grid.tiles[index].counter++;

                w0 += delta_w0_x;
                w1 += delta_w1_x;
                w2 += delta_w2_x;
            }
            w0_y += delta_w0_y;
            w1_y += delta_w1_y;
            w2_y += delta_w2_y;
        }
    }

    int last_offset {0};
    for (auto& tile : grid.tiles)
    {
        tile.offset = last_offset;
        tile.cursor_offset = last_offset;
        last_offset = tile.counter + tile.offset;
    }

    // Could be grid.triangles_id.size, but I don't want to be casting here
    if (grid.last_tri_count < last_offset)
    {
        grid.triangles_id.resize(last_offset);
        grid.last_tri_count = last_offset;
    }

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
            const auto index = grid_max_x + grid.width * grid_max_y;
            grid.triangles_id[grid.tiles[index].cursor_offset] = static_cast<int>(id);
            grid.tiles[index].cursor_offset++;
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
                grid.triangles_id[grid.tiles[index].cursor_offset] = static_cast<int>(id);
                grid.tiles[index].cursor_offset++;

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

Color* Viewport::frame_buffer_data() noexcept
{
    return frame_buffer.data();
}

void Viewport::update_frame_buffer_size(const int w, const  int h) noexcept
{
    width = w/res_factor;
    height = h/res_factor;
    half_width = static_cast<float>(width) / 2;
    half_height = static_cast<float>(height) / 2;
    frame_buffer.resize(width*height, BLACK);
    depth_buffer.resize(width*height, 0.0f);

    update_tiles();
}

Vec3 Viewport::ndc_to_screen(const Vec3 &point) const
{
    return
    {
        (point.x + 1.0f) * half_width,
        (1.0f - point.y) * half_height,
        point.z
    };
}

float Viewport::aspect_ratio() const
{
    return static_cast<float>(width) / static_cast<float>(height);
}

bool Viewport::depth_pass(const int x, const int y, const float z_depth) noexcept
{
    const int index = y * width + x;
    if (z_depth > depth_buffer[index]) return false;

    depth_buffer[index] = z_depth;
    return true;
}

void Viewport::put_pixel(const int x, const int y, const Vec4 &color) noexcept
{
    frame_buffer[y * width + x] = color_convertion::vec4_to_color(color);
}