#include "material/texture.h"

#include <cmath>

#include "transforms/constants.h"

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
    const auto [tl, tr, bl, br, wx, wy] {bilinear_coord(uv)};

    const auto texel_top_left {rgb_to_linear[buffer_value[tl]]};
    const auto texel_top_right {rgb_to_linear[buffer_value[tr]]};
    const auto texel_bottom_left {rgb_to_linear[buffer_value[bl]]};
    const auto texel_bottom_right {rgb_to_linear[buffer_value[br]]};

    const auto top_row {transforms::linear_interpolation(texel_top_left, texel_top_right, wx)};
    const auto bottom_row {transforms::linear_interpolation(texel_bottom_left, texel_bottom_right, wx)};

    return transforms::linear_interpolation(top_row, bottom_row, wy);
}

Vec4 TextureRaster::bilinear_color(const Vec2 uv) const noexcept
{
    const auto [tl, tr, bl, br, wx, wy] {bilinear_coord(uv)};
    const auto texel_top_left {color_to_vec4(buffer[tl])};
    const auto texel_top_right {color_to_vec4(buffer[tr])};
    const auto texel_bottom_left {color_to_vec4(buffer[bl])};
    const auto texel_bottom_right {color_to_vec4(buffer[br])};

    const auto top_row {texel_top_left.interpolate(texel_top_right, wx)};
    const auto bottom_row {texel_bottom_left.interpolate(texel_bottom_right, wx)};

    return top_row.interpolate(bottom_row, wy);
}

Vec4 TextureRaster::bilinear_color_gamma(const Vec2 uv) const noexcept
{
    const auto [tl, tr, bl, br, wx, wy] {bilinear_coord(uv)};

    const auto texel_top_left {color_to_vec4_gamma(buffer[tl])};
    const auto texel_top_right {color_to_vec4_gamma(buffer[tr])};
    const auto texel_bottom_left {color_to_vec4_gamma(buffer[bl])};
    const auto texel_bottom_right {color_to_vec4_gamma(buffer[br])};

    const auto top_row {texel_top_left.interpolate(texel_top_right, wx)};
    const auto bottom_row {texel_bottom_left.interpolate(texel_bottom_right, wx)};

    return top_row.interpolate(bottom_row, wy);
}