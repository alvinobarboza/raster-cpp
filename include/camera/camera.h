#pragma once
#include <vector>

#include "frustum.h"
#include "raylib.h"
#include "model/triangle.h"
#include "transforms/transforms.h"
#include "transforms/vec2.h"


class CameraRaster {
    float fov_angle {};
    float fov_scale {};
    float aspect_ratio {};
    float z_near {};
    float z_far {};
    float sensitivity {};

    bool update_view {};

    void update_frustum();
    void move_forward_backwards(float unit);
    void move_left_right(float unit);
    void move_up_down(float unit);
    void update_rotation(const Vec2 &rotation);
public:
    Transforms transform {};
    Matrix4x4 projection_matrix {};
    Frustum frustum {};

    CameraRaster(
        float sensitivity, float fov,
        float near, float far,
        const Vec3 &position, const Vec3 &rotation);

    [[nodiscard]] Vec3 vertex_to_ndc(const Vec3 &vertex) const;

    void update_aspect_ratio(float new_aspect_ratio);
    void toggle_view_lock();
    void handle_input();
};

static float fov_scaling(float angle);