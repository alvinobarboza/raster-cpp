#pragma once
#include "scene.h"

class RendererRaster {
    // Sutherland–Hodgman tmp vars
    std::vector<Vertex> verts_in = {};
    std::vector<Vertex> verts_out = {};

    std::vector<FullTriangle> tris_buffer = {};

    // just near and far for now
    void clip_triangle(const Plane& near, const Plane& far);

    static void render_triangle(const FullTriangle &tri, const SceneRaster &scene);
public:
    RendererRaster() = default;
    void render_scene(const SceneRaster& scene);
};
