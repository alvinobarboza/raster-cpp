#pragma once
#include "scene.h"

class RendererRaster {
    static void render_triangle(const FullTriangle &tri, const SceneRaster &scene);
public:
    static void render_scene(const SceneRaster& scene);
};
