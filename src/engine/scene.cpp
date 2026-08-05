#include "engine/scene.h"

SceneRaster::SceneRaster(CameraRaster &camera, const SkyBox &skybox)
    :
    camera(camera),
    skybox(skybox)
{}
