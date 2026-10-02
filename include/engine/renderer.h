#pragma once
#include <atomic>
#include <thread>

#include "scene.h"
#include "timer.h"
#include "viewport.h"

struct G_buffer {
    float depth{1.0f};
    int triangle_id{-1};
    Vec2 uv {};
    Vec3 normal {};
};

enum class RenderMode {
    FORWARD_TILED_M,
    DEFERRED_TILED_M,
    SHADOW_MAPPING,

    MAX_VALUE
};

enum class RenderStage {
    SHADOW,
    CAMERA,
};

enum class ShadingMode {
    DEPTH,
    ALBEDO,
    NORMAL,
    LIT
};

class RendererRaster {
    bool render_light {};
    bool render_depth {};
    bool render_normal {};
    bool render_wireframe {};
    bool render_triangle_aabb {};
    bool render_active_tiles {};
    RenderMode render_mode { RenderMode::DEFERRED_TILED_M };
    RenderStage render_stage { RenderStage::CAMERA };
    ShadingMode shading_mode { ShadingMode::ALBEDO };
    int shadow_index {0};

    SceneRaster* scene {nullptr};
    Light* current_light {nullptr}; // Just for now, let's see if I find a better implementation

    std::vector<TimerSample> profile_samples {};

    // Sutherland–Hodgman tmp vars
    std::vector<Vertex> verts_in {};
    std::vector<Vertex> verts_out {};

    std::vector<Vec3> verts_in_sm {}; // for shadow mapping
    std::vector<Vec3> verts_out_sm {}; // for shadow mapping

    std::vector<std::jthread> workers{};
    std::vector<FullTriangle> t_camera_buffer {};
    std::vector<ShadowTriangle> t_shadow_buffer {};

    alignas(64) std::atomic<int> frame_counter{0};
    alignas(64) std::atomic<int> tile_index{0};
    alignas(64) std::atomic<int> active_workers{0};
    alignas(64) std::atomic<bool> stop_flag{false};

    // just near and far for now
    void clip_triangle(const Plane& near) noexcept;
    void clip_triangle_sm(const Plane& near) noexcept; // for shadows

    static bool is_outside_ndc(const Vec3& ndc0, const Vec3& ndc1, const Vec3& ndc2) noexcept;
    static bool is_outside_screen(const Vec3& sc1, const Vec3& sc2, const Vec3& sc3, float w, float h) noexcept;

    void draw_line(Vec3 a, Vec3 b) noexcept;
    void draw_aabb(const AABB2D& aabb) noexcept;
    void draw_wireframe_triangle(const FullTriangle& triangle) noexcept;
    void draw_wireframe_from_tri_buffer() noexcept;
    void draw_triangle_aabb() noexcept;

    template<ShadingMode mode>
    void render_tile_deferred(const Tile& tile, std::span<G_buffer> g_buffer) noexcept;

    void render_tile_forward(const Tile& tile) noexcept;

    void draw_active_tiles() noexcept;

    void woke_threads() noexcept;
    void render_multithread() noexcept;

    void shadow_mapping() noexcept;
    void render_shadow_map_tile(const Tile &tile) const noexcept;
    void render_shadow_map(const Light& value) noexcept;
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
    void cycle_shadow_index();

    [[nodiscard]] std::span<const TimerSample> time_samples() const noexcept;
    [[nodiscard]] std::string renderer_mode() const noexcept;

    void handle_input();
};
