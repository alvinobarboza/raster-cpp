#pragma once
#include <string>

#include "texture.h"
#include "transforms/Vec3.h"

// Simplified
class MaterialRaster {
public:
    std::string name = "default";

    Vec3 diffuse   = {}; // Kd
    float specular = 0.0f; // Ns

    const TextureRaster* map_diffuse   = nullptr; // map_kd
    const TextureRaster* map_normal    = nullptr; // map_Bump
    const TextureRaster* map_roughness = nullptr; // map_Pr / map_Ns

    MaterialRaster(
        std::string name,
        Vec3 diffuse_color, float specular,
        const TextureRaster *diffuse,
        const TextureRaster *normal,
        const TextureRaster *roughness);
};
