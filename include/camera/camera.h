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

    std::size_t width = 0;
    std::size_t height = 0;

    float half_width = 0.0f;
    float half_height = 0.0f;

    Transforms transform = {};

    Frustum frustum = {};

    CameraRaster(
        std::size_t width, std::size_t height,
        float sensitivity, float fov,
        Vec3 position, Vec3 rotation);

    void update_frame_buffer_size();
    void clear_frame_buffer();
    [[nodiscard]] Vec2 vertex_to_ndc(const Vec3 &vertex) const;
    [[nodiscard]] Vec3 ndc_to_screen(const Vec2 &point) const;
    [[nodiscard]] FullTriangle project_triangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        Texture &albedo,
        Texture &normal,
        Texture &specular) const;
};
