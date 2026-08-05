#pragma once
#include "camera/camera.h"
#include "model/mesh.h"
#include "transforms/vec4.h"

struct SkyBox {
    MaterialRaster* material;
    Vec4 ambient_color;
};

class SceneRaster {
public:
    CameraRaster& camera;
    std::vector<ModelRaster*> models = {};
    const SkyBox skybox;

    SceneRaster(CameraRaster& camera, const SkyBox& skybox);
};
