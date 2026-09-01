#pragma once
#include "camera/camera.h"
#include "lights/light.h"
#include "model/mesh.h"
#include "transforms/vec4.h"

struct SkyBox {
    MaterialRaster* material {};
    Vec4 ambient_color {};
    float ambient_intensity { 1.0f };
};

class SceneRaster {
public:
    CameraRaster& camera;
    std::vector<ModelRaster*> models {};
    std::vector<Light> lights {};
    const SkyBox skybox;

    SceneRaster(CameraRaster& camera, const SkyBox& skybox);
};
