#pragma once
#include <atomic>
#include <thread>

#include "scene.h"
#include "viewport.h"

struct Gbuffer {
    Vec3 frag_coord {};
    Vec3 albedo {};
    Vec3 normal {};
    float roughness {};
    float depth{1e5f};
};

enum class RenderMode {
    FORWARD,
    FORWARD_TILED,
    DEFERRED_TILED,
    FORWARD_TILED_M,
    DEFERRED_TILED_M,

    MAX_VALUE
};

class RendererRaster {
    bool render_light {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};
    bool render_triangle_aabb {};
    bool render_active_tiles {};
    RenderMode render_mode {RenderMode::FORWARD};

    SceneRaster* scene {nullptr};

    // Sutherland–Hodgman tmp vars
    std::vector<Vertex> verts_in {};
    std::vector<Vertex> verts_out {};

    std::vector<std::jthread> workers{};
    std::vector<FullTriangle> tris_buffer {};

    alignas(64) std::atomic<int> frame_counter{0};
    alignas(64) std::atomic<int> tile_index{0};
    alignas(64) std::atomic<int> active_workers{0};
    alignas(64) std::atomic<bool> stop_flag{false};

    // just near and far for now
    void clip_triangle(const Plane& near, const Plane& far) noexcept;

    static bool is_outside_screen(const Vec3& ndc0, const Vec3& ndc1, const Vec3& ndc2) noexcept;

    void draw_line(Vec3 a, Vec3 b) noexcept;
    void draw_aabb(const AABB2D& aabb) noexcept;
    void draw_wireframe_triangle(const FullTriangle& triangle) noexcept;
    void draw_wireframe_from_tri_buffer() noexcept;
    void draw_triangle_aabb() noexcept;

    void render_triangle(const FullTriangle &tri) noexcept;

    void render_tile_deferred(const Tile& tile, std::span<Gbuffer> g_buffer) noexcept;
    void render_tile_forward(const Tile& tile) noexcept;

    void render_tiles_deferred() noexcept;
    void render_tiles_forward() noexcept;
    void draw_active_tiles() noexcept;

    void woke_threads() noexcept;
    void render_multithread() noexcept;
public:
    Viewport viewport {};

    explicit RendererRaster(int w, int h, int res_factor) noexcept;
    ~RendererRaster() noexcept;

    void render_scene(SceneRaster* s);

    void toggle_wireframe();
    void toggle_render_depth();
    void toggle_render_normal();
    void toggle_render_light();
    void toggle_render_triangle_aabb();
    void toggle_render_active_tiles();
    void toggle_render_mode();

    [[nodiscard]] std::string renderer_mode() const noexcept;

    void handle_input();
};
