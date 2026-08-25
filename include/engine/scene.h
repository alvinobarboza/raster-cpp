#pragma once
#include "camera/camera.h"
#include "lights/light.h"
#include "model/mesh.h"
#include "transforms/vec4.h"

struct SkyBox {
    MaterialRaster* material = nullptr;
    Vec4 ambient_color {};
};

class SceneRaster {
public:
    CameraRaster& camera;
    std::vector<ModelRaster*> models = {};
    std::vector<Light> lights = {};
    const SkyBox skybox;

    SceneRaster(CameraRaster& camera, const SkyBox& skybox);
};
