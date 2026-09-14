#pragma once
#include <vector>
#include "raylib.h"

#include "transforms/vec2.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

class Viewport {
    std::vector<Color> frame_buffer {};
    std::vector<float> depth_buffer {};

    int res_factor {};

    float half_width {};
    float half_height {};
public:
    int width {};
    int height {};

    Viewport () = default;
    Viewport(int width, int height, int res_factor);

    [[nodiscard]] Color* frame_buffer_data() noexcept;
    void clear_frame_buffer();
    void update_frame_buffer_size(int w, int h);
    [[nodiscard]] Vec2 ndc_to_screen(const Vec3 &point) const;
    [[nodiscard]] float aspect_ratio() const;
    bool depth_pass(int x, int y, float z_depth);
    void put_pixel(int x, int y, const Vec4 &color);
};
