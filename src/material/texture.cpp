#include "material/texture.h"

#include <cmath>

vec4 TextureRaster::texel_color(const vec2 &uv) const
{
    const int w = static_cast<int>(std::floor(uv.x * f_width)) & width_mask;
    const int h = static_cast<int>(std::floor(uv.y * f_height)) & height_mask;
    return buffer[h * width + w];
}

float TextureRaster::texel_intensity(const vec2 &uv) const
{
    return texel_color(uv).x;
}
