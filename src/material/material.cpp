#include "material/material.h"

#include <utility>

MaterialRaster::MaterialRaster(
    std::string name, const Vec4 diffuse_color,
    const float specular,
    const TextureRaster *diffuse,
    const TextureRaster *normal,
    const TextureRaster *roughness):
name(std::move(name)), diffuse(diffuse_color), specular(specular),
map_diffuse(diffuse), map_normal(normal), map_roughness(roughness) {}
