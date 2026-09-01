#pragma once
#include <string>

#include "texture.h"
#include "transforms/vec3.h"

// Simplified
class MaterialRaster {
public:
    std::string name { "default" };

    Vec4 diffuse   {-1.0f, -1.0f, -1.0f, 1.0f}; // Kd
    float specular { -1.0f }; // Ns

    const TextureRaster* map_diffuse   {}; // map_kd
    const TextureRaster* map_normal    {}; // map_Bump
    const TextureRaster* map_roughness {}; // map_Pr / map_Ns

    MaterialRaster() = default;

    MaterialRaster(
        std::string name,
        Vec4 diffuse_color, float specular,
        const TextureRaster *diffuse,
        const TextureRaster *normal,
        const TextureRaster *roughness);
};
