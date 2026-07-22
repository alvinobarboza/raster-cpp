#pragma once
#include <vector>

#include "frustum.h"
#include "raylib.h"
#include "mesh/triangle.h"
#include "transforms/Transforms.h"
#include "transforms/Vec2.h"


class CameraRaster {
public:
    std::vector<Color> frame_buffer = {};
    std::vector<float> depth_buffer = {};

    float fov_angle = 0.0f;
    float fov_scale = 0.0f;
    float aspect_ratio = 0.0f;
    float z_near = 0.0f;
    float z_far = 0.0f;
    float sensitivity = 0.0f;

    bool update_view = true;
    bool render_depth = false;
    bool render_wireframe = false;

    int width = 0;
    int height = 0;

    float half_width = 0.0f;
    float half_height = 0.0f;

    Transforms transform = {};

    Frustum frustum = {};

    CameraRaster(
        int width, int height,
        float sensitivity, float fov,
        float near, float far,
        const Vec3 &position, const Vec3 &rotation);

    void update_frame_buffer_size(int w, int h);
    void clear_frame_buffer();
    [[nodiscard]] Vec2 vertex_to_ndc(const Vec3 &vertex) const;
    [[nodiscard]] Vec2 ndc_to_screen(const Vec2 &point) const;
    [[nodiscard]] FullTriangle project_triangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        TextureRaster *albedo,
        TextureRaster *normal,
        TextureRaster *specular) const;
    [[nodiscard]] bool depth_pass(int x, int y, float depth);
    void put_pixel(int x, int y, const Vec4 &color, float depth);
    void move_forward_backwards(float unit);
    void move_left_right(float unit);
    void move_up_down(float unit);
    void update_rotation(const Vec2 &rotation);
    void toggle_view_lock();
    void toggle_wireframe();
    void toggle_render_depth();
    void update_frustum();
};

static float fov_scaling(float angle);