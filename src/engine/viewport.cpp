#include <algorithm>

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
    // JUST TO SAVE TESTS
    // int tile_size = 4;
    // int width = 14;
    // int height = 8;
    // int tile_x = (width + tile_size - 1) / tile_size;
    // int tile_y = (height + tile_size - 1) / tile_size;
    // int total_tiles = tile_x * tile_y;
    //
    //
    // char l = 'a';
    // std::vector<char> screen (width*height, l);
    //
    // print("tile_size", tile_size);
    // std::cout << width << " x " << height << '\n';
    // print("tile_x", tile_x);
    // print("tile_y", tile_y);
    // print("total_tiles", total_tiles);
    // print("mult x", tile_x * tile_size);
    // print("mult y", tile_y * tile_size);
    //
    // int offset_x = 0;
    // int offset_y = 0;
    //
    // for(int ty = 0; ty < tile_y; ++ty)
    // {
    //     for(int tx = 0; tx < tile_x; ++tx)
    //     {
    //         //std::cout << "\n[off_x: " << offset_x;
    //         //std::cout << " off_y: " << offset_y;
    //         //std::cout << "] \n";
    //         for(int y = 0; y < tile_size; ++y)
    //         {
    //             for(int x = 0; x < tile_size; ++x)
    //             {
    //                 const auto fx = x + offset_x;
    //                 const auto fy = y + offset_y;
    //                 const auto index = fx + fy * width;
    //                 //std::cout << "[x: " << fx;
    //                 //std::cout << " fy: " << fy;
    //                 //std::cout << " i: " << index;
    //                 //std::cout << "] ";
    //                 if (fx < width && fy < height) {
    //                     screen[index] = l;
    //                 }
    //             }
    //             //std::cout << '\n';
    //         }
    //         l++;
    //         offset_x += tile_size;
    //     }
    //     //std::cout << '\n';
    //     offset_y += tile_size;
    //     offset_x = 0;
    // }
    // std::cout << '\n';
    // //end:

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