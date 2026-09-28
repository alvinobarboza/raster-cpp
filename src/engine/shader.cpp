#include "engine/shader.h"

#include <algorithm>
#include <cmath>

#include "transforms/constants.h"

float shader::distributionGGX(const float NdotH, const float roughness) noexcept
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f ;
    denom = transforms::PI_R * denom * denom;
    return a2 / std::ranges::max(denom, 0.0000001f); // Prevent divide by zero
}

float shader::geometrySmith(const float NdotV, const float NdotL, const float roughness) noexcept
{
    const float r = roughness + 1.0f;
    const float k = r * r / 8.0f;
    const float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
    const float ggx2 = NdotL / (NdotL * (1.0f - k) + k);
    return ggx1 * ggx2;
}

Vec3 shader::fresnelSchlick(const float HdotV, const Vec3& baseReflectivity) noexcept
{
    const Vec3 inverse_reflectivity {1.0f - baseReflectivity.x, 1.0f - baseReflectivity.y, 1.0f - baseReflectivity.z};
    const auto t1 = 1.0f - HdotV;
    const auto t = t1*t1*t1*t1*t1;
    return baseReflectivity + inverse_reflectivity * t;
}

Vec4 shader::calculate_light(
    const std::vector<Light>& lights,
    const float frag_rough,
    const Vec3& frag_pos,
    const Vec4& frag_color,
    const Vec3& frag_normal,
    const Vec3& view_normal,
    const float ambient_intensity) noexcept
{
    const Vec3 albedo = {
        std::pow(frag_color.x, 2.2f),
        std::pow(frag_color.y, 2.2f),
        std::pow(frag_color.z, 2.2f)
    };

    // If I add metallic property, lerp from 0.04 to albedo/diffuse using the range[0,1] of metallic
    const Vec3 base_reflectivity {0.04};

    Vec3 Lo {};
    for (const auto& light : lights)
    {
        const float d {frag_normal * light.direction_view_space};
        if ( d <= 0.0f) continue;

        if (light.has_shadows)
        {
            const Vec3 frag_light_pos {frag_pos * light.project_view_matrix }; //TODO: try to interpolate frag position in the triangle stage
            const float depth_light {(frag_light_pos.z + 1.0f) * 0.5f};
            constexpr float bias_depth {0.0008f};
            const Vec2 uv_light_coord {(frag_light_pos.x + 1.0f) * 0.5f, (1.0f - frag_light_pos.y) * 0.5f};

            if (const auto shadow {light.shadow.sample(uv_light_coord)}; frag_light_pos.z <= 1.0f && (depth_light - bias_depth) > shadow)
            {
                continue;
            }
        }

        // Also (light_pos - frag_pos) for point light
        const Vec3 L = light.direction_view_space;
        const Vec3 H = (view_normal + L).normalized();

        // This a direction light, no attenuation will be applied now
        /*
         *  float distance = length(light_pos - frag_pos);
         *  float attenuation = 1.0 / (distance * distance);
         *  vec3 radiance = ligth_color * attenuation;
         */
        const Vec3 radiance {light.color.x*light.intensity, light.color.y*light.intensity, light.color.z*light.intensity};

        // Cook-Torrance BRDF
        const float NdotV = std::max(frag_normal * view_normal, 0.0000001f);
        const float NdotL = std::max(d, 0.0000001f);
        const float HdotV = std::max(H * view_normal, 0.0f);
        const float NdotH = std::max(frag_normal * H, 0.0f);

        const float D = distributionGGX(NdotH, frag_rough);
        const float G = geometrySmith(NdotV, NdotL, frag_rough);
        const Vec3 F = fresnelSchlick(HdotV, base_reflectivity);

        const Vec3 specular = F * D * G / (4.0f * NdotV * NdotL);

        const Vec3 kD = Vec3{1.0f} - F;

        // When using metallic property
        // kD *= 1.0 - metallic;

        // Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        const Vec3 kD_x_albedo {kD.x * albedo.x, kD.y * albedo.y, kD.z * albedo.z};
        constexpr float INV_PI {1.0f / transforms::PI_R};
        const Vec3 divided_pi_specular = kD_x_albedo * INV_PI + specular;
        const Vec3 mul_radiance {
            divided_pi_specular.x * radiance.x,
            divided_pi_specular.y * radiance.y,
            divided_pi_specular.z * radiance.z};

        Lo += mul_radiance * NdotL;
    }

    const Vec3 ambient {albedo * ambient_intensity};
    Vec3 color = ambient + Lo;

    // HDR tone mapping
    const float lum = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
    color = color / (1.0f + lum);
    // Gamma
    constexpr float gamma_const {1.0f/2.2f};

    color = {
        std::pow(color.x, gamma_const),
        std::pow(color.y, gamma_const),
        std::pow(color.z, gamma_const)
    };

    return {
        std::clamp(color.x, 0.0f, 1.0f),
        std::clamp(color.y, 0.0f, 1.0f),
        std::clamp(color.z, 0.0f, 1.0f),
        1.0f
    };
}