#pragma once
#include <vector>
#include "raylib.h"
#include "screen_tile.h"
#include "model/triangle.h"

#include "transforms/vec3.h"
#include "transforms/vec4.h"

class Viewport {
    std::vector<Color> frame_buffer {};
    std::vector<float> depth_buffer {};

    int res_factor {};

    float half_width {};
    float half_height {};
public:
    static constexpr int TILE_SIZE = 32;
    std::vector<ScreenTile> tiles{};
    int width {};
    int height {};

    Viewport () = default;
    Viewport(int width, int height, int res_factor);

    [[nodiscard]] Color* frame_buffer_data() noexcept;
    void clear_frame_buffer() noexcept;
    void update_tiles() noexcept;
    void reset_tiles() noexcept;
    void update_frame_buffer_size(int w, int h) noexcept;
    [[nodiscard]] Vec3 ndc_to_screen(const Vec3 &point) const;
    [[nodiscard]] float aspect_ratio() const;
    bool depth_pass(int x, int y, float z_depth) noexcept;
    void put_pixel(int x, int y, const Vec4 &color) noexcept;
};
