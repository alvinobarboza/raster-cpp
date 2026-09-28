#pragma once
#include <vector>

#include "lights/light.h"
#include "transforms/vec3.h"
#include "transforms/vec4.h"

/*
 * Brian Will -> ?v=5p0e7YNONr8
 */

namespace shader
{
    float distributionGGX(float NdotH, float roughness) noexcept;
    float geometrySmith(float NdotV, float NdotL, float roughness) noexcept;
    Vec3 fresnelSchlick(float HdotV, const Vec3& baseReflectivity) noexcept;
    Vec4 calculate_light(
        const std::vector<Light>& lights,
        float frag_rough,
        const Vec3& frag_pos,
        const Vec4& frag_color,
        const Vec3& frag_normal,
        const Vec3& view_normal,
        float ambient_intensity) noexcept;
}