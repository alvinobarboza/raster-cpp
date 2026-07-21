#include "material/texture.h"

#include <cmath>

Vec4 Texture::texel_color(const Vec2 &uv) const
{
    const int w = static_cast<int>(std::floor(uv.x * f_width)) & width_mask;
    const int h = static_cast<int>(std::floor(uv.y * f_height)) & height_mask;
    return buffer[h * width + w];
}

float Texture::texel_intensity(const Vec2 &uv) const
{
    return texel_color(uv).x;
}
