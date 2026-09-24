#pragma once
#include "types.h"
#include "transforms/transforms.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

class Light {
    public:
    LightType type {};
    Vec4 color {};
    float intensity {};

    Vec3 direction_view_space {};
    Transforms transform {};

    Matrix4x4 projection_matrix {}; // from matrix generation - to_orthographic or to_perspective
    Matrix4x4 project_view_matrix {}; // camera_transform x light_transform x light_projection

    Light() = default;
    explicit Light(
        LightType type,
        const Vec4 &color,
        float intensity,
        const Vec3 &rotation,
        const Vec3& position) noexcept;
};
