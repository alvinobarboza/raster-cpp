#include <algorithm>

#include "engine/viewport.h"
#include "material/color_convertion.h"

Viewport::Viewport(
    const int width, const int height, const int res_factor): res_factor(res_factor)
{
    update_frame_buffer_size(width, height);
}

void Viewport::clear_frame_buffer()
{
    std::ranges::fill(frame_buffer, BLACK);
    std::ranges::fill(depth_buffer, 0.0f);
}

Color* Viewport::frame_buffer_data() noexcept
{
    return frame_buffer.data();
}

void Viewport::update_frame_buffer_size(const int w, const  int h)
{
    width = w/res_factor;
    height = h/res_factor;
    half_width = static_cast<float>(width) / 2;
    half_height = static_cast<float>(height) / 2;
    frame_buffer.resize(width*height, BLACK);
    depth_buffer.resize(width*height, 0.0f);
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

bool Viewport::depth_pass(const int x, const int y, const float z_depth)
{
    const int index = y * width + x;
    if (z_depth < depth_buffer[index]) return false;

    depth_buffer[index] = z_depth;
    return true;
}

void Viewport::put_pixel(const int x, const int y, const Vec4 &color)
{
    frame_buffer[y * width + x] = color_convertion::vec4_to_color(color);
}