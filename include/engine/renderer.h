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
    static constexpr int TILE_SIZE = 32;

    bool render_light {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};
    bool render_triangle_aabb {};

    // Sutherland–Hodgman tmp vars
    std::vector<Vertex> verts_in {};
    std::vector<Vertex> verts_out {};

    std::vector<FullTriangle> tris_buffer {};

    // just near and far for now
    void clip_triangle(const Plane& near, const Plane& far) noexcept;

    static bool is_outside_screen(const Vec3& ndc0, const Vec3& ndc1, const Vec3& ndc2) noexcept;

    void draw_line(Vec3 a, Vec3 b) noexcept;
    void draw_aabb(const AABB2D& aabb) noexcept;
    void draw_wireframe_triangle(const FullTriangle& triangle) noexcept;
    void draw_wireframe_from_tri_buffer() noexcept;
    void draw_triangle_aabb() noexcept;
    void render_triangle(const FullTriangle &tri, const SceneRaster &scene) noexcept;
public:
    Viewport viewport {};

    explicit RendererRaster(int w, int h, int res_factor) noexcept;
    void render_scene(SceneRaster& scene);

    void toggle_wireframe();
    void toggle_render_depth();
    void toggle_render_normal();
    void toggle_render_light();
    void toggle_render_triangle_aabb();

    void handle_input();
};
