#pragma once
#include "shadow.h"
#include "types.h"
#include "camera/frustum.h"
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

    Frustum frustum {};
    ShadowMap shadow {};
    bool has_shadows {false};

    Light() = default;
    explicit Light(
        LightType type,
        const Vec4 &color,
        float intensity,
        const Vec3 &rotation,
        const Vec3& position, bool shadows = false) noexcept;

    Vec3 vertex_to_ndc(const Vec3& point) const noexcept;
    Vec3 ndc_to_canvas(const Vec3 &point) const noexcept;
};
