#pragma once
#include <memory>
#include <vector>

#include "model/mesh.h"

class ResourceManager {
    std::vector<MaterialRaster> load_material(const std::string& path);
    TextureRaster* load_texture(const std::string& path);
public:
    std::vector<std::unique_ptr<ModelRaster>> models = {};
    std::vector<std::unique_ptr<TextureRaster>> textures = {};

    ModelRaster* load_model(const std::string& path, bool flip_handiness);
    std::vector<ModelRaster*> load_scene(const std::string& path);
};
