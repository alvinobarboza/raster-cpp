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

    tiles.clear();
    tiles.reserve(total_tiles);

    std::cout << "viewport_res: " << width << 'x' << height << '\n';
    std::cout << "tile_size: " << TILE_SIZE << '\n';
    std::cout << "tile_x: " << tile_x << '\n';
    std::cout << "tile_y: " << tile_y << '\n';
    std::cout << "total_tiles: " << total_tiles << '\n';

    AABB2D aabb_temp;

    for(int ty = 0; ty < tile_y; ++ty)
    {
        for(int tx = 0; tx < tile_x; ++tx)
        {
            const float offset_x = static_cast<float>(tx) * TILE_SIZE;
            const float offset_y = static_cast<float>(ty) * TILE_SIZE;

            aabb_temp.min.x = offset_x;
            aabb_temp.min.y = offset_y;
            aabb_temp.max.x = offset_x+TILE_SIZE-1;
            aabb_temp.max.y = offset_y+TILE_SIZE-1;

            tiles.emplace_back(aabb_temp);
            //std::cout << aabb_temp << '\n';
        }
    }
}

void Viewport::reset_tiles() noexcept
{
    std::ranges::for_each(tiles, [](ScreenTile& tile)
    {
        tile.is_active = false;
        tile.triangles_id.clear();
    });
}

void Viewport::bin_triangles(const std::vector<FullTriangle> &triangles) noexcept
{
    for (const auto [index, triangle] : std::views::enumerate(triangles))
    {
        for (auto&[aabb, triangles_id, is_active] : tiles)
        {
            if (triangle.aabb.collides(aabb))
            {
                is_active = true;
                triangles_id.push_back(index);
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