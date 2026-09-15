#pragma once
#include "scene.h"
#include "viewport.h"

class alignas(32) Gbuffer {
public:
    Vec3 albedo {};
    Vec3 normal {};
    float roughness {};
    float depth{};
};

class RendererRaster {
    bool render_light {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};

    // Sutherland–Hodgman tmp vars
    std::vector<Vertex> verts_in {};
    std::vector<Vertex> verts_out {};

    std::vector<FullTriangle> tris_buffer {};

    // just near and far for now
    void clip_triangle(const Plane& near, const Plane& far) noexcept;

    void render_triangle(const FullTriangle &tri, const SceneRaster &scene) noexcept;
public:
    Viewport viewport {};

    explicit RendererRaster(int w, int h, int res_factor) noexcept;
    void render_scene(SceneRaster& scene);

    void toggle_wireframe();
    void toggle_render_depth();
    void toggle_render_normal();
    void toggle_render_light();

    void handle_input();
};
