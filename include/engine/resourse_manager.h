#pragma once
#include <memory>
#include <vector>

#include "model/mesh.h"

class ResourceManager {
public:
    std::vector<std::unique_ptr<ModelRaster>> models = {};
    std::vector<std::unique_ptr<TextureRaster>> textures = {};

    ModelRaster* load_model(const std::string& path);
    MaterialRaster load_material(const std::string& path);
    TextureRaster* load_texture(const std::string& path);
    std::vector<ModelRaster*> load_scene(const std::string& path);
};
