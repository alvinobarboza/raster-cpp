#pragma once
#include "types.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

class Light {
    public:
    LightType type {};
    Vec4 color {};
    float intensity {};

    Vec3 direction {};
    Vec3 direction_view_space {};

    Light() = default;
    Light(const LightType type, const Vec4 &color, const float intensity, const Vec3 &direction):
        type(type), color(color), intensity(intensity), direction(direction)
    {};
};
