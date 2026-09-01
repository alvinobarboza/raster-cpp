#pragma once
#include <vector>

#include "frustum.h"
#include "raylib.h"
#include "model/triangle.h"
#include "transforms/transforms.h"
#include "transforms/vec2.h"


class CameraRaster {
public:
    std::vector<Color> frame_buffer {};
    std::vector<float> depth_buffer {};

    float fov_angle {};
    float fov_scale {};
    float aspect_ratio {};
    float z_near {};
    float z_far {};
    float sensitivity {};

    bool update_view {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};

    int width {};
    int height {};
    int res_factor {};

    float half_width {};
    float half_height {};

    Transforms transform {};
    Matrix4x4 projection_matrix {};

    Frustum frustum {};

    CameraRaster(
        int width, int height, int res_factor,
        float sensitivity, float fov,
        float near, float far,
        const Vec3 &position, const Vec3 &rotation);

    void update_frame_buffer_size(int w, int h);
    void clear_frame_buffer();
    [[nodiscard]] Vec3 vertex_to_ndc(const Vec3 &vertex) const;
    [[nodiscard]] Vec2 ndc_to_screen(const Vec3 &point) const;
    [[nodiscard]] FullTriangle project_triangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        const MaterialRaster& material) const;
    [[nodiscard]] bool depth_pass(int x, int y, float depth);
    void put_pixel(int x, int y, const Vec4 &color);
    void move_forward_backwards(float unit);
    void move_left_right(float unit);
    void move_up_down(float unit);
    void update_rotation(const Vec2 &rotation);
    void toggle_view_lock();
    void toggle_wireframe();
    void toggle_render_depth();
    void toggle_render_normal();
    void update_frustum();
    void handle_input();
};

static float fov_scaling(float angle);