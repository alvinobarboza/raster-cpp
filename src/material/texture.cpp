#include "material/texture.h"

#include <cmath>

#include "transforms/constants.h"

Vec2 TextureRaster::texel_coord(const Vec2 uv) const noexcept
{
    const float inverted_y = 1.0f - uv.y;
    const auto xf = uv.x * f_width;
    const auto yf = inverted_y * f_height;
    const auto floor_x = static_cast<float>(static_cast<int>(xf) & width_mask);
    const auto floor_y = static_cast<float>(static_cast<int>(yf) & height_mask);
    return {
        floor_x + (xf - floor_x),
        floor_y + (yf - floor_y),
    };
}

Vec4 TextureRaster::texel_color(const Vec2 uv) const noexcept
{
    const auto coord = texel_coord(uv);
    const int w = static_cast<int>(coord.x);
    const int h = static_cast<int>(coord.y);
    return buffer[h * width + w];
}

Vec3 TextureRaster::texel_normal(const Vec2 uv) const noexcept
{
    const auto normal = bilinear_color(uv);

    return {
        normal.x * 2 - 1,
        normal.y * 2 - 1,
        normal.z * 2 - 1,
    };
}

float TextureRaster::texel_intensity(const Vec2 uv) const noexcept
{
    const Vec2 tex_coord {texel_coord(uv)};

    const auto floor_x {std::floor(tex_coord.x)};
    const auto floor_y {std::floor(tex_coord.y)};
    const Vec2 f{floor_x, floor_y};

    const Vec2 weights {tex_coord.x - floor_x, tex_coord.y - floor_y};
    const int base_x {static_cast<int>(f.x)};
    const int base_y {static_cast<int>(f.y)};

    // x,y offset
    const int max_index {static_cast<int>(buffer_float.size())};
    const int index_tl {base_y * width + base_x}; //0,0 offset
    const int index_tr {base_y * width + (base_x + 1)}; //1,0 offset
    const int index_bl {(base_y + 1) * width + base_x}; //0,1 offset
    const int index_br {(base_y + 1) * width + (base_x + 1)}; //1,1 offset

    const auto texel_top_left {buffer_float[index_tl < max_index ? index_tl : max_index - 1]};
    const auto texel_top_right {buffer_float[index_tr < max_index ? index_tr : max_index - 1]};
    const auto texel_bottom_left {buffer_float[index_bl < max_index ? index_bl : max_index - 1]};
    const auto texel_bottom_right {buffer_float[index_br < max_index ? index_br : max_index - 1]};

    const auto top_row {transforms::lerp(texel_top_left, texel_top_right, weights.x)};
    const auto bottom_row {transforms::lerp(texel_bottom_left, texel_bottom_right, weights.x)};

    return transforms::lerp(top_row, bottom_row, weights.y);
}

Vec4 TextureRaster::bilinear_color(const Vec2 uv) const noexcept
{
    const Vec2 tex_coord {texel_coord(uv)};

    const auto floor_x {std::floor(tex_coord.x)};
    const auto floor_y {std::floor(tex_coord.y)};
    const Vec2 f{floor_x, floor_y};

    const Vec2 weights {tex_coord.x - floor_x, tex_coord.y - floor_y};
    const int base_x {static_cast<int>(f.x)};
    const int base_y {static_cast<int>(f.y)};

    // x,y offset
    const int max_index {static_cast<int>(buffer.size())};
    const int index_tl {base_y * width + base_x}; //0,0 offset
    const int index_tr {base_y * width + (base_x + 1)}; //1,0 offset
    const int index_bl {(base_y + 1) * width + base_x}; //0,1 offset
    const int index_br {(base_y + 1) * width + (base_x + 1)}; //1,1 offset

    const auto texel_top_left {buffer[index_tl < max_index ? index_tl : max_index - 1]};
    const auto texel_top_right {buffer[index_tr < max_index ? index_tr : max_index - 1]};
    const auto texel_bottom_left {buffer[index_bl < max_index ? index_bl : max_index - 1]};
    const auto texel_bottom_right {buffer[index_br < max_index ? index_br : max_index - 1]};

    const auto top_row {texel_top_left.lerp_to(texel_top_right, weights.x)};
    const auto bottom_row {texel_bottom_left.lerp_to(texel_bottom_right, weights.x)};

    return top_row.lerp_to(bottom_row, weights.y);
}
