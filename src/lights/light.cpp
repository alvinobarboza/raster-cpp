#include "lights/light.h"

Light::Light(
    const LightType type,
    const Vec4 &color,
    const float intensity,
    const Vec3 &rotation,
    const Vec3 &position) noexcept: type(type), color(color),intensity(intensity)
{
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = {1,1,1};
    transform.forward_direction = {0,0,1};

    transform.update_transforms(true);

    if (type == LightType::DIRECTIONAL)
    {
        projection_matrix.to_orthographic(
            -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 30.0f);
    }
}
