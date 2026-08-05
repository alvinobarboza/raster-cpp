#pragma once
#include <memory>
#include <vector>

#include "model/mesh.h"

class ResourceManager {
public:
    std::vector<std::unique_ptr<ModelRaster>> models = {};
    std::vector<std::unique_ptr<TextureRaster>> textures = {};

    ModelRaster* LoadModel(const std::string& path);
    MaterialRaster* LoadMaterial(const std::string& path);
    std::vector<ModelRaster*> LoadScene(const std::string& path);
};
