#pragma once
#include "scene.h"
#include "viewport.h"

class alignas(16) Gbuffer {
public:
    Vec3 frag_coord {};
    float roughness {};
    Vec4 albedo {};
    Vec3 normal {};
    float depth{1e5f};
};

class RendererRaster {
    bool render_light {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};
    bool render_triangle_aabb {};
    bool render_active_tiles {};
    bool render_forward { true };
    bool render_deferred {};

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
    void render_tiles(const SceneRaster &scene) noexcept;
    void draw_active_tiles() noexcept;
public:
    Viewport viewport {};

    explicit RendererRaster(int w, int h, int res_factor) noexcept;
    void render_scene(SceneRaster& scene);

    void toggle_wireframe();
    void toggle_render_depth();
    void toggle_render_normal();
    void toggle_render_light();
    void toggle_render_triangle_aabb();
    void toggle_render_active_tiles();
    void toggle_render_forward();
    void toggle_render_deferred();

    void handle_input();
};
