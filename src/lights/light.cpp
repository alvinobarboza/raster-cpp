#include "lights/light.h"

Light::Light(
    const LightType type,
    const Vec4 &color,
    const float intensity,
    const Vec3 &rotation,
    const Vec3 &position, const bool shadows) noexcept:
type(type),
color(color),
intensity(intensity),
shadow(shadows ? ShadowMap():ShadowMap{0,0}),
has_shadows(shadows)
{

    transform.position = position;
    transform.rotation = rotation;
    transform.scale = {1,1,1};
    transform.forward_direction = {0,0,1};

    transform.update_transforms(true);

    if (type == LightType::DIRECTIONAL)
    {
        constexpr float box_size {5.0f};
        constexpr float left { -box_size };
        constexpr float right { box_size };
        constexpr float bottom { -box_size };
        constexpr float top { box_size };
        constexpr float near { 1.0f };
        constexpr float far { 30.0f };

        projection_matrix.to_orthographic(
            left, right, bottom, top, near, far);

        const Vec3 near_n {0.0f, 0.0f,1.0f};
        const Vec3 far_n {0.0f, 0.0f,-1.0f};
        const Vec3 left_n {1.0f, 0.0f,0.0f};
        const Vec3 right_n {-1.0f, 0.0f,0.0f};
        const Vec3 bottom_n {0.0f, 1.0f,0.0f};
        const Vec3 top_n {0.0f, -1.0f,0.0f};

        frustum.planes[NEAR_PLANE] = {near, near_n};
        frustum.planes[FAR_PLANE] = {-far, far_n};
        frustum.planes[LEFT_PLANE] = {left, left_n};
        frustum.planes[RIGHT_PLANE] = {-right, right_n};
        frustum.planes[BOTTOM_PLANE] = {bottom, bottom_n};
        frustum.planes[TOP_PLANE] = {-top, top_n};
    }
}

Vec3 Light::vertex_to_ndc(const Vec3& point) const noexcept
{
    const Vec4 v4 = {point.x, point.y, point.z, 1.0f};
    const Vec4 clip = v4 * projection_matrix;

    return {
        clip.x,
        clip.y,
        (clip.z + 1.0f) * 0.5f // normalizing to 0 <> 1
    };
}

Vec3 Light::ndc_to_canvas(const Vec3 &point) const noexcept
{
    return
    {
        (point.x + 1.0f) * static_cast<float>(shadow.half_width),
        (1.0f - point.y) * static_cast<float>(shadow.half_height),
        point.z
    };
}