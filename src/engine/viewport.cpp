#include <algorithm>
#include <ranges>

#include "engine/viewport.h"
#include "material/color_convertion.h"

Viewport::Viewport(
    const int width, const int height, const int res_factor): res_factor(res_factor)
{
    update_frame_buffer_size(width, height);
}

void Viewport::clear_frame_buffer() noexcept
{
    std::ranges::fill(frame_buffer, BLACK);
    std::ranges::fill(depth_buffer, 0.0f);
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

void Viewport::bin_triangles(const std::vector<FullTriangle> &triangles) noexcept
{
    for (const FullTriangle &triangle : triangles)
    {
        const auto tri_min_y = std::max(static_cast<int>(triangle.aabb.min.y), 0);
        const auto tri_max_y = std::min(static_cast<int>(triangle.aabb.max.y), height - 1);
        const auto tri_min_x = std::max(static_cast<int>(triangle.aabb.min.x), 0);
        const auto tri_max_x = std::min(static_cast<int>(triangle.aabb.max.x), width - 1);

        const int grid_min_x {tri_min_x / TILE_SIZE}, grid_max_x {tri_max_x / TILE_SIZE};
        const int grid_min_y {tri_min_y / TILE_SIZE}, grid_max_y {tri_max_y / TILE_SIZE};

        for (int y = grid_min_y; y <= grid_max_y; ++y)
        {
            for (int x = grid_min_x; x <= grid_max_x; ++x)
            {
                // TODO: discard tiles that are outside of the triangle completely
                // by calculating edge_cross: All fours w0 < 0.0f or all fours w1 < 0.0f or w2 < 0.0f
                // by basically check the 3 edges
                const auto index = x + grid.width * y;
                grid.tiles[index].counter++;
            }
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

    for (const auto [id, triangle] : std::views::enumerate(triangles))
    {
        const auto tri_min_y = std::max(static_cast<int>(triangle.aabb.min.y), 0);
        const auto tri_max_y = std::min(static_cast<int>(triangle.aabb.max.y), height - 1);
        const auto tri_min_x = std::max(static_cast<int>(triangle.aabb.min.x), 0);
        const auto tri_max_x = std::min(static_cast<int>(triangle.aabb.max.x), width - 1);

        const int grid_min_x {tri_min_x / TILE_SIZE}, grid_max_x {tri_max_x / TILE_SIZE};
        const int grid_min_y {tri_min_y / TILE_SIZE}, grid_max_y {tri_max_y / TILE_SIZE};

        for (int y = grid_min_y; y <= grid_max_y; ++y)
        {
            for (int x = grid_min_x; x <= grid_max_x; ++x)
            {
                const auto index = x + grid.width * y;
                grid.triangles_id[grid.tiles[index].cursor_offset] = static_cast<int>(id);
                grid.tiles[index].cursor_offset++;
            }
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
    if (z_depth < depth_buffer[index]) return false;

    depth_buffer[index] = z_depth;
    return true;
}

void Viewport::put_pixel(const int x, const int y, const Vec4 &color) noexcept
{
    frame_buffer[y * width + x] = color_convertion::vec4_to_color(color);
}