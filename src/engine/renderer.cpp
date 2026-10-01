#include "engine/renderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "engine/shader.h"
#include "engine/timer.h"
#include "material/color_convertion.h"

RendererRaster::RendererRaster(const int w, const int h, const int res_factor) noexcept:
viewport(w, h, res_factor)
{
    const auto threads = std::thread::hardware_concurrency();
    workers.reserve(threads);
    for (unsigned int i = 0; i < threads; ++i)
    {
        workers.emplace_back(&RendererRaster::render_multithread, this);
    }
}

RendererRaster::~RendererRaster() noexcept
{
    stop_flag.store(true);
    frame_counter.fetch_add(1);
    frame_counter.notify_all();
}

void RendererRaster::clip_triangle(const Plane& near) noexcept
{
    std::swap(verts_in, verts_out);
    verts_out.clear();

    size_t prev_index = verts_in.size() - 1;
    for (size_t i = 0; i < verts_in.size(); i++)
    {
        const auto current_point = verts_in[i];
        const auto [point, normal, uv] = verts_in[prev_index];

        const auto distance_current_point = near.signed_distance_to_point(current_point.point);
        const auto distance_prev_point = near.signed_distance_to_point(point);

        if (distance_current_point > 0.0f)
        {
            if (distance_prev_point <= 0.0f)
            {
                const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
                verts_out.emplace_back(
                    current_point.point.interpolate(point, ratio),
                    current_point.normal.interpolate(normal, ratio),
                    current_point.uv.interpolate(uv, ratio));
            }
            verts_out.push_back(current_point);
        }
        else if (distance_prev_point > 0)
        {
            const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
            verts_out.emplace_back(
                current_point.point.interpolate(point, ratio),
                current_point.normal.interpolate(normal, ratio),
                current_point.uv.interpolate(uv, ratio));
        }
        prev_index = i;
    }
}

void RendererRaster::clip_triangle_sm(const Plane& near) noexcept
{
    std::swap(verts_in_sm, verts_out_sm);
    verts_out_sm.clear();

    size_t prev_index = verts_in_sm.size() - 1;
    for (size_t i = 0; i < verts_in_sm.size(); i++)
    {
        const auto current_point = verts_in_sm[i];
        const auto point = verts_in_sm[prev_index];

        const auto distance_current_point = near.signed_distance_to_point(current_point);
        const auto distance_prev_point = near.signed_distance_to_point(point);

        if (distance_current_point > 0.0f)
        {
            if (distance_prev_point <= 0.0f)
            {
                const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
                verts_out_sm.emplace_back(current_point.interpolate(point, ratio));
            }
            verts_out_sm.push_back(current_point);
        }
        else if (distance_prev_point > 0)
        {
            const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
            verts_out_sm.emplace_back(current_point.interpolate(point, ratio));
        }
        prev_index = i;
    }
}

bool RendererRaster::is_outside_screen(const Vec3 &ndc0, const Vec3 &ndc1, const Vec3 &ndc2) noexcept
{
    //UP
    if (ndc0.y > 1.0f && ndc1.y > 1.0f && ndc2.y > 1.0f) return true;
    //DOWN
    if (ndc0.y < -1.0f && ndc1.y < -1.0f && ndc2.y < -1.0f) return true;
    //LEFT
    if (ndc0.x < -1.0f && ndc1.x < -1.0f && ndc2.x < -1.0f) return true;
    //RIGHT
    if (ndc0.x > 1.0f && ndc1.x > 1.0f && ndc2.x > 1.0f) return true;

    return false;
}

void RendererRaster::render_scene(SceneRaster* const s)
{
    scene = s;
    viewport.clear_frame_buffer();
    viewport.reset_tiles();
    tris_buffer.clear();

    if (scene == nullptr)
    {
        std::cout << "shouldn't be null here \n";
        return;
    }

    //update lights
    constexpr Matrix4x4 light_bias_matrix {
        0.5f,  0.0f,  0.0f,  0.5f,
        0.0f, -0.5f,  0.0f,  0.5f,
        0.0f,  0.0f,  0.5f,  0.5f,
        0.0f,  0.0f,  0.0f,  1.0f,
    };

    for (auto &light: scene->lights)
    {
        // Since this is used only for the dot product between the light and triangle normal, I'm inverting here
        // Normal UP * actual light direction, will always produce negative value for a correct light setup.
        if (light.type == LightType::DIRECTIONAL)
        {
            const auto light_target = ((
                scene->camera.transform.forward_direction *
                scene->camera.transform.rotation_matrix) * light.ortho_size) + scene->camera.transform.position;

            const auto light_world_dir = light.transform.forward_direction * light.transform.rotation_matrix;
            light.transform.position = light_target - light_world_dir * light.focus_distance;
            light.transform.update_transforms(true);

            light.direction_view_space = -(light_world_dir * scene->camera.transform.transposed_rotation_matrix).normalized();
            light.project_view_matrix = light_bias_matrix *
                    light.projection_matrix * light.transform.view_matrix * scene->camera.transform.world_matrix;
            shadow_mapping(light);
        }
    }

    for (const auto& model: scene->models)
    {
        const auto m_transforms = scene->camera.transform.view_matrix * model->transforms.world_matrix;
        model->boundingSphere.center_view_space = model->boundingSphere.center * m_transforms;
        model->to_render = scene->camera.frustum.is_inside_frustum(model->boundingSphere);
    }

    std::ranges::sort(scene->models, []( ModelRaster*& a, ModelRaster*& b) {
        return a->boundingSphere.center_view_space.length() > b->boundingSphere.center_view_space.length();
    });

    for (const auto& model : scene->models)
    {
        if (!model->to_render)
        {
            //std::cout << "[SKIP] " << model->name << "\n";
            continue;
        }

        const auto m_rotation = scene->camera.transform.transposed_rotation_matrix * model->transforms.rotation_matrix;
        const auto m_transforms = scene->camera.transform.view_matrix * model->transforms.world_matrix;

        for (size_t i = 0; i < model->meshData.vertices.size(); ++i)
        {
            model->meshData.vertices_view_space[i] = model->meshData.vertices[i] * m_transforms;
        }

        for (size_t i = 0; i < model->meshData.normals.size(); ++i)
        {
            model->meshData.normals_view_space[i] = model->meshData.normals[i] * m_rotation;
        }

        for (const auto &t: model->meshData.triangles)
        {
            const auto p0_d {scene->camera.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v1])};
            const auto p1_d {scene->camera.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v2])};
            const auto p2_d {scene->camera.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v3])};

            if (p0_d <= 0.0f && p1_d <= 0.0f && p2_d <= 0.0f) continue;

            const auto normal {t.normal * m_rotation};

            if ((normal * -model->meshData.vertices_view_space[t.v1]) <= 0.0f) continue;

            const auto tangent {t.tangent * m_rotation};

            verts_out.clear();
            verts_in.clear();

            verts_out.emplace_back(
                model->meshData.vertices_view_space[t.v1],
                model->meshData.normals_view_space[t.n1],
                model->meshData.uvs[t.u1]);

            verts_out.emplace_back(
                model->meshData.vertices_view_space[t.v2],
                model->meshData.normals_view_space[t.n2],
                model->meshData.uvs[t.u2]);

            verts_out.emplace_back(
                model->meshData.vertices_view_space[t.v3],
                model->meshData.normals_view_space[t.n3],
                model->meshData.uvs[t.u3]);


            if (!(p0_d >= 0.0f && p1_d >= 0.0f && p2_d >= 0.0f))
            {
                clip_triangle(scene->camera.frustum.planes[NEAR_PLANE]);
            }

            if (verts_out.size() > 2) {
                for (size_t j = 1; j < verts_out.size() - 1; ++j) {
                    const auto p1 = verts_out[0];
                    const auto p2 = verts_out[j];
                    const auto p3 = verts_out[j + 1];

                    const auto ndc0 = scene->camera.vertex_to_ndc(p1.point);
                    const auto ndc1 = scene->camera.vertex_to_ndc(p2.point);
                    const auto ndc2 = scene->camera.vertex_to_ndc(p3.point);

                    if (is_outside_screen(ndc0, ndc1, ndc2))
                    {
                        //++count_skipped_tris;
                        continue;
                    }

                    const auto tangent {t.tangent * m_rotation};

                    FullTriangle tf {
                        p1,p2,p3,
                        model->meshData.materials[t.material_id],
                        normal, tangent,
                        t.smooth
                    };

                    tf.screen_points[0] = viewport.ndc_to_screen(ndc0);
                    tf.screen_points[1] = viewport.ndc_to_screen(ndc1);
                    tf.screen_points[2] = viewport.ndc_to_screen(ndc2);

                    tf.calculate_tri_aabb();

                    tris_buffer.push_back(tf);
                }
            }
        }
    }

    if (render_mode == RenderMode::DEFERRED_TILED_M || render_mode == RenderMode::FORWARD_TILED_M)
    {
        viewport.bin_triangles(tris_buffer);
        woke_threads();
    }
    else if (render_mode == RenderMode::SHADOW_MAPPING)
    {
        const auto& light = scene->lights[shadow_index];
        render_shadow_map(light);
    }

    if (render_wireframe)
    {
        draw_wireframe_from_tri_buffer();
    }

    if (render_triangle_aabb)
    {
        draw_triangle_aabb();
    }

    if (render_active_tiles)
    {
        draw_active_tiles();
    }
}

void RendererRaster::woke_threads() noexcept
{
    tile_index.store(0);
    active_workers.store(workers.size());

    frame_counter.fetch_add(1);
    frame_counter.notify_all();

    int current = active_workers.load();
    while (current > 0) {
        active_workers.wait(current);
        current = active_workers.load();
    }
}

void RendererRaster::render_tile_deferred(const Tile& tile, std::span<G_buffer> g_buffer) noexcept
{
    const int max_offset_y = std::min(tile.offset_y+Viewport::TILE_SIZE, viewport.height);
    const int max_offset_x = std::min(tile.offset_x+Viewport::TILE_SIZE, viewport.width);

    for (const auto triangle_id : tile.triangles_id)
    {
        const auto& tri = tris_buffer[triangle_id];

        const auto min_y = std::max(static_cast<int>(tri.aabb.min.y), tile.offset_y);
        const auto max_y = std::min(static_cast<int>(tri.aabb.max.y), max_offset_y);
        const auto min_x = std::max(static_cast<int>(tri.aabb.min.x), tile.offset_x);
        const auto max_x = std::min(static_cast<int>(tri.aabb.max.x), max_offset_x);

        const auto delta_w0_col = tri.screen_points[1].y - tri.screen_points[2].y;
        const auto delta_w1_col = tri.screen_points[2].y - tri.screen_points[0].y;
        const auto delta_w2_col = tri.screen_points[0].y - tri.screen_points[1].y;

        const auto delta_w0_row = tri.screen_points[2].x - tri.screen_points[1].x;
        const auto delta_w1_row = tri.screen_points[0].x - tri.screen_points[2].x;
        const auto delta_w2_row = tri.screen_points[1].x - tri.screen_points[0].x;

        const auto bias_0 {triangle::is_edge_top_or_left(tri.screen_points[1], tri.screen_points[2])};
        const auto bias_1 {triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0])};
        const auto bias_2 {triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1])};

        const auto cross = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
        if (cross < 1e-6f) continue; // possible edge case
        const auto area = 1.0f / cross;
        const Vec3 p = {static_cast<float>(min_x) + 0.5f, static_cast<float>(min_y) + 0.5f, 0.0f};

        auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p);
        auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p);
        auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p);

        // Deltas to step over all attributes (ndc_depth, z_depth, uv, normal, frag_pos)
        const auto delta_alpha_x { delta_w0_col * area };
        const auto delta_beta_x { delta_w1_col * area };
        const auto delta_gamma_x { delta_w2_col * area };

        const auto delta_alpha_y { delta_w0_row * area };
        const auto delta_beta_y { delta_w1_row * area };
        const auto delta_gamma_y { delta_w2_row * area };

        // Attributes deltas
        const auto delta_ndc_z_x { tri.screen_points[0].z * delta_alpha_x + tri.screen_points[1].z * delta_beta_x + tri.screen_points[2].z * delta_gamma_x };
        const auto delta_ndc_z_y { tri.screen_points[0].z * delta_alpha_y + tri.screen_points[1].z * delta_beta_y + tri.screen_points[2].z * delta_gamma_y };

        const auto delta_frag_z_x { tri.depth_z[0] * delta_alpha_x + tri.depth_z[1] * delta_beta_x + tri.depth_z[2] * delta_gamma_x };
        const auto delta_frag_z_y { tri.depth_z[0] * delta_alpha_y + tri.depth_z[1] * delta_beta_y + tri.depth_z[2] * delta_gamma_y };

        const auto delta_frag_uv_x { tri.projected_vertices[0].uv * delta_alpha_x + tri.projected_vertices[1].uv * delta_beta_x + tri.projected_vertices[2].uv * delta_gamma_x };
        const auto delta_frag_uv_y { tri.projected_vertices[0].uv * delta_alpha_y + tri.projected_vertices[1].uv * delta_beta_y + tri.projected_vertices[2].uv * delta_gamma_y };

        const auto delta_frag_normal_x { tri.projected_vertices[0].normal * delta_alpha_x + tri.projected_vertices[1].normal * delta_beta_x + tri.projected_vertices[2].normal * delta_gamma_x };
        const auto delta_frag_normal_y { tri.projected_vertices[0].normal * delta_alpha_y + tri.projected_vertices[1].normal * delta_beta_y + tri.projected_vertices[2].normal * delta_gamma_y };

        // Attributes first values
        auto ndc_z_row { (tri.screen_points[0].z * w0_row + tri.screen_points[1].z * w1_row + tri.screen_points[2].z * w2_row) * area };
        auto frag_z_row { (tri.depth_z[0] * w0_row + tri.depth_z[1] * w1_row + tri.depth_z[2] * w2_row) * area };
        auto frag_uv_row { (tri.projected_vertices[0].uv * w0_row + tri.projected_vertices[1].uv * w1_row + tri.projected_vertices[2].uv * w2_row) * area  };
        auto frag_normal_row { (tri.projected_vertices[0].normal * w0_row + tri.projected_vertices[1].normal * w1_row + tri.projected_vertices[2].normal * w2_row) * area  };

        for (int y = min_y; y < max_y; y++)
        {
            auto w0 = w0_row;
            auto w1 = w1_row;
            auto w2 = w2_row;

            auto z_depth { ndc_z_row };
            auto frag_z { frag_z_row };
            auto frag_uv_over_z { frag_uv_row };
            auto frag_normal_over_z { frag_normal_row };

            int row_index = (y - tile.offset_y) * Viewport::TILE_SIZE + (min_x - tile.offset_x);
            for (int x = min_x; x < max_x; x++)
            {
                const auto w0_check = bias_0 ? w0 >= 0.0f : w0 > 0.0f;
                const auto w1_check = bias_1 ? w1 >= 0.0f : w1 > 0.0f;
                const auto w2_check = bias_2 ? w2 >= 0.0f : w2 > 0.0f;

                if (w0_check && w1_check && w2_check)
                {
                    if (g_buffer[row_index].depth > z_depth)
                    {
                        const auto frag_depth { 1 / frag_z };
                        const auto frag_uv { frag_uv_over_z * frag_depth };
                        const auto frag_normal { frag_normal_over_z * frag_depth };

                        g_buffer[row_index].triangle_id = triangle_id;
                        g_buffer[row_index].depth = z_depth;
                        g_buffer[row_index].uv = frag_uv;
                        g_buffer[row_index].normal = frag_normal;
                    }
                }
                w0 += delta_w0_col;
                w1 += delta_w1_col;
                w2 += delta_w2_col;

                z_depth += delta_ndc_z_x;
                frag_z += delta_frag_z_x;
                frag_uv_over_z += delta_frag_uv_x;
                frag_normal_over_z += delta_frag_normal_x;

                ++row_index;
            }
            w0_row += delta_w0_row;
            w1_row += delta_w1_row;
            w2_row += delta_w2_row;

            ndc_z_row += delta_ndc_z_y;
            frag_z_row += delta_frag_z_y;
            frag_uv_row += delta_frag_uv_y;
            frag_normal_row += delta_frag_normal_y;
        }
    }

    const auto z_near { scene->camera.z_near };
    const auto z_far { scene->camera.z_far };
    const auto aspect_ratio { scene->camera.aspect_ratio };
    const auto fov_scale { scene->camera.fov_scale };

    for (int y = 0; y < Viewport::TILE_SIZE; y++)
    {
        for (int x = 0; x < Viewport::TILE_SIZE; x++)
        {
            const int index = x + y * Viewport::TILE_SIZE;
            const auto& gb = g_buffer[index];
            if (gb.triangle_id < 0) continue;

            const auto px = tile.offset_x + x;
            const auto py = tile.offset_y + y;

            if (px >= viewport.width || py >= viewport.height) continue;

            viewport.depth_pass(px, py, gb.depth);
            const auto& t {tris_buffer[gb.triangle_id]};

            // Reconstruct frag_coord
            const auto p_center_x = static_cast<float>(px) + 0.5f;
            const auto p_center_y = static_cast<float>(py) + 0.5f;

            const auto z_view { ( z_near * z_far ) / ( z_far - ( gb.depth * ( z_far - z_near ) ) ) };
            const auto x_ndc { ( p_center_x - viewport.half_width ) / viewport.half_width };
            const auto y_ndc { ( viewport.half_height - p_center_y ) / viewport.half_height };
            const auto x_view { x_ndc * z_view * ( aspect_ratio / fov_scale ) };
            const auto y_view { y_ndc * z_view * ( 1.0f / fov_scale ) };

            const Vec3 frag_coord { x_view, y_view, z_view };
            const Vec3 view_normal {-x_view, -y_view, -z_view};
            const auto albedo { t.frag_color(gb.uv) };
            const auto normal { t.frag_normal(gb.normal.normalized(), gb.uv) };
            const auto roughness { t.frag_roughness(gb.uv) };

            if (render_normal)
            {
                const Vec4 final_color{
                    normal.x * .5f + .5f,
                    normal.y * .5f + .5f,
                    -normal.z * .5f + .5f,
                    1.0f
                };

                viewport.put_pixel(px, py, final_color);
                continue;
            }
            if (render_depth)
            {
                const Vec4 final_color{
                    1-gb.depth,
                    1-gb.depth,
                    1-gb.depth,
                    1.0f
                };
                viewport.put_pixel(px, py, final_color);
                continue;
            }
            if (!render_light)
            {
                const Vec4 final_color{
                    std::sqrt(albedo.x),
                    std::sqrt(albedo.y),
                    std::sqrt(albedo.z),
                    1.0f};
                viewport.put_pixel(px, py, final_color);
                continue;
            }

            const auto final_color = shader::calculate_light(
                scene->lights, roughness, frag_coord, albedo,
                normal, view_normal.normalized(), scene->skybox.ambient_intensity);

            viewport.put_pixel(px, py, final_color);
        }
    }
}

void RendererRaster::render_tile_forward(const Tile &tile) noexcept
{
        const int max_offset_y = std::min(tile.offset_y+Viewport::TILE_SIZE, viewport.height);
        const int max_offset_x = std::min(tile.offset_x+Viewport::TILE_SIZE, viewport.width);
        for (const auto triangle_id : tile.triangles_id)
        {
            const auto& tri = tris_buffer[triangle_id];

            const auto min_y = std::max(static_cast<int>(tri.aabb.min.y), tile.offset_y);
            const auto max_y = std::min(static_cast<int>(tri.aabb.max.y), max_offset_y);
            const auto min_x = std::max(static_cast<int>(tri.aabb.min.x), tile.offset_x);
            const auto max_x = std::min(static_cast<int>(tri.aabb.max.x), max_offset_x);

            const auto delta_w0_col = tri.screen_points[1].y - tri.screen_points[2].y;
            const auto delta_w1_col = tri.screen_points[2].y - tri.screen_points[0].y;
            const auto delta_w2_col = tri.screen_points[0].y - tri.screen_points[1].y;

            const auto delta_w0_row = tri.screen_points[2].x - tri.screen_points[1].x;
            const auto delta_w1_row = tri.screen_points[0].x - tri.screen_points[2].x;
            const auto delta_w2_row = tri.screen_points[1].x - tri.screen_points[0].x;

            const auto bias_0 {triangle::is_edge_top_or_left(tri.screen_points[1], tri.screen_points[2])};
            const auto bias_1 {triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0])};
            const auto bias_2 {triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1])};

            const auto cross = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
            if (cross < 1e-6f) continue; // possible edge case
            const auto area = 1.0f / cross;
            const Vec3 p = {static_cast<float>(min_x) + 0.5f, static_cast<float>(min_y) + 0.5f, 0.0f};

            auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p);
            auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p);
            auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p);

            for (int y = min_y; y < max_y; y++)
            {
                auto w0 = w0_row;
                auto w1 = w1_row;
                auto w2 = w2_row;

                for (int x = min_x; x < max_x; x++)
                {
                    const auto w0_check = bias_0 ? w0 >= 0.0f : w0 > 0.0f;
                    const auto w1_check = bias_1 ? w1 >= 0.0f : w1 > 0.0f;
                    const auto w2_check = bias_2 ? w2 >= 0.0f : w2 > 0.0f;

                    if (w0_check && w1_check && w2_check)
                    {
                        const auto alpha = w0 * area;
                        const auto beta = w1 * area;
                        const auto gamma = w2 * area;

                        if (const float z_depth = tri.frag_depth_ndc(alpha, beta, gamma);
                            viewport.depth_pass(x, y, z_depth))
                        {
                            const auto frag_depth = 1 / tri.frag_depth(alpha, beta, gamma);
                            const auto frag_uv = tri.frag_uv_coord(alpha, beta, gamma, frag_depth);
                            const auto frag_coord = tri.frag_coord(alpha, beta, gamma, frag_depth);
                            const auto frag_normal = tri.frag_normal(alpha, beta, gamma, frag_uv,frag_depth);
                            const auto frag_color = tri.frag_color(frag_uv);
                            const float frag_rough = tri.frag_roughness(frag_uv);

                            if (render_normal)
                            {
                                const Vec4 final_color{
                                    frag_normal.x * .5f + .5f,
                                    frag_normal.y * .5f + .5f,
                                    -frag_normal.z * .5f + .5f,
                                    1.0f
                                };

                                viewport.put_pixel(x, y, final_color);
                            }
                            else if (render_depth)
                            {
                                float c = 1-z_depth;
                                if (c < 0.01) c = 0.01;

                                const Vec4 final_color {
                                    c,c,c,1.0f
                                };

                                viewport.put_pixel(x, y, final_color);
                            }
                            else if (render_light)
                            {
                                const auto view_normal = (-frag_coord).normalized();
                                const auto final_color = shader::calculate_light(
                                    scene->lights, frag_rough, frag_coord, frag_color, frag_normal,
                                    view_normal, scene->skybox.ambient_intensity);

                                viewport.put_pixel(x, y, final_color);
                            }
                            else
                            {
                                const Vec4 final_color{
                                    std::sqrt(frag_color.x),
                                    std::sqrt(frag_color.y),
                                    std::sqrt(frag_color.z),
                                    1.0f};
                                viewport.put_pixel(x, y, final_color);
                            }
                        }
                    }
                    w0 += delta_w0_col;
                    w1 += delta_w1_col;
                    w2 += delta_w2_col;
                }
                w0_row += delta_w0_row;
                w1_row += delta_w1_row;
                w2_row += delta_w2_row;
            }
        }
}

void RendererRaster::shadow_mapping(Light& light) noexcept
{
    if (!light.has_shadows) return;

    light.shadow.clear();

    for (const auto& model: scene->models)
    {
        const auto m_transforms = light.transform.view_matrix * model->transforms.world_matrix;
        model->boundingSphere.center_view_space = model->boundingSphere.center * m_transforms;
        model->to_render = light.frustum.is_inside_frustum(model->boundingSphere);
    }

    std::ranges::sort(scene->models, []( ModelRaster*& a, ModelRaster*& b) {
        return a->boundingSphere.center_view_space.length() < b->boundingSphere.center_view_space.length();
    });

    for (const auto& model : scene->models)
    {
        if (!model->to_render)
        {
            continue;
        }

        const auto m_rotation = light.transform.transposed_rotation_matrix * model->transforms.rotation_matrix;
        const auto m_transforms = light.transform.view_matrix * model->transforms.world_matrix;

        for (size_t i = 0; i < model->meshData.vertices.size(); ++i)
        {
            model->meshData.vertices_view_space[i] = model->meshData.vertices[i] * m_transforms;
        }

        for (const auto &t: model->meshData.triangles)
        {
            const auto p0_d {light.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v1])};
            const auto p1_d {light.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v2])};
            const auto p2_d {light.frustum.planes[NEAR_PLANE].signed_distance_to_point(model->meshData.vertices_view_space[t.v3])};

            if (p0_d <= 0.0f && p1_d <= 0.0f && p2_d <= 0.0f) continue;

            if (const auto normal {t.normal * m_rotation};
                (normal * -model->meshData.vertices_view_space[t.v1]) <= 0.0f) continue;

            verts_out_sm.clear();
            verts_in_sm.clear();

            verts_out_sm.emplace_back(model->meshData.vertices_view_space[t.v1]);
            verts_out_sm.emplace_back(model->meshData.vertices_view_space[t.v2]);
            verts_out_sm.emplace_back(model->meshData.vertices_view_space[t.v3]);

            if (!(p0_d >= 0.0f && p1_d >= 0.0f && p2_d >= 0.0f))
            {
                clip_triangle_sm(light.frustum.planes[NEAR_PLANE]);
            }

            if (verts_out_sm.size() > 2) {
                for (size_t j = 1; j < verts_out_sm.size() - 1; ++j) {
                    const auto p1 = verts_out_sm[0];
                    const auto p2 = verts_out_sm[j];
                    const auto p3 = verts_out_sm[j + 1];

                    const auto ndc0 = light.vertex_to_ndc(p1);
                    const auto ndc1 = light.vertex_to_ndc(p2);
                    const auto ndc2 = light.vertex_to_ndc(p3);

                    if (is_outside_screen(ndc0, ndc1, ndc2))
                    {
                        continue;
                    }

                    ShadowTriangle sf {};

                    sf.screen_points[0] = light.ndc_to_canvas(ndc0);
                    sf.screen_points[1] = light.ndc_to_canvas(ndc1);
                    sf.screen_points[2] = light.ndc_to_canvas(ndc2);

                    sf.calculate_tri_aabb();
                    render_shadow_map_triangle(light, sf);
                }
            }
        }
    }
}

void RendererRaster::render_shadow_map_triangle(Light& light, const ShadowTriangle &tri) noexcept
{
    const auto min_y = std::max(tri.aabb.min.y, 0.0f);
    const auto max_y = std::min(tri.aabb.max.y, static_cast<float>(light.shadow.height));
    const auto min_x = std::max(tri.aabb.min.x, 0.0f);
    const auto max_x = std::min(tri.aabb.max.x, static_cast<float>(light.shadow.width));

    const auto delta_w0_col = tri.screen_points[1].y - tri.screen_points[2].y;
    const auto delta_w1_col = tri.screen_points[2].y - tri.screen_points[0].y;
    const auto delta_w2_col = tri.screen_points[0].y - tri.screen_points[1].y;

    const auto delta_w0_row = tri.screen_points[2].x - tri.screen_points[1].x;
    const auto delta_w1_row = tri.screen_points[0].x - tri.screen_points[2].x;
    const auto delta_w2_row = tri.screen_points[1].x - tri.screen_points[0].x;

    const auto bias_0 {triangle::is_edge_top_or_left(tri.screen_points[1], tri.screen_points[2])};
    const auto bias_1 {triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0])};
    const auto bias_2 {triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1])};

    const auto area = 1.0f / triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
    const Vec3 p = {min_x + 0.5f, min_y + 0.5f, 0.0f};

    auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p);
    auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p);
    auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p);

    // Deltas to step over all ndc_depth
    const auto delta_alpha_x { delta_w0_col * area };
    const auto delta_beta_x { delta_w1_col * area };
    const auto delta_gamma_x { delta_w2_col * area };

    const auto delta_alpha_y { delta_w0_row * area };
    const auto delta_beta_y { delta_w1_row * area };
    const auto delta_gamma_y { delta_w2_row * area };

    // Attribute delta
    const auto delta_ndc_z_x { tri.screen_points[0].z * delta_alpha_x + tri.screen_points[1].z * delta_beta_x + tri.screen_points[2].z * delta_gamma_x };
    const auto delta_ndc_z_y { tri.screen_points[0].z * delta_alpha_y + tri.screen_points[1].z * delta_beta_y + tri.screen_points[2].z * delta_gamma_y };

    // Attribute first value
    auto ndc_z_row { (tri.screen_points[0].z * w0_row + tri.screen_points[1].z * w1_row + tri.screen_points[2].z * w2_row) * area };

    for (int y = static_cast<int>(min_y); y < max_y; y++)
    {
        auto w0 { w0_row };
        auto w1 { w1_row };
        auto w2 { w2_row };
        auto z_depth { ndc_z_row };

        for (int x = static_cast<int>(min_x); x < max_x; x++)
        {
            const auto w0_check = bias_0 ? w0 >= 0.0f : w0 > 0.0f;
            const auto w1_check = bias_1 ? w1 >= 0.0f : w1 > 0.0f;
            const auto w2_check = bias_2 ? w2 >= 0.0f : w2 > 0.0f;

            if (w0_check && w1_check && w2_check)
            {
                light.shadow.depth_test(x, y, z_depth);
            }

            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;

            z_depth += delta_ndc_z_x;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;

        ndc_z_row += delta_ndc_z_y;
    }
}

void RendererRaster::render_shadow_map(const Light &value) noexcept {
    const int min_value = std::min(viewport.width, viewport.height);
    const int max_value = std::max(viewport.width, viewport.height);
    const int offset = (max_value - min_value) / 2;
    const float delta = 1.0f/static_cast<float>(min_value);

    Vec2 uv{};
    for (int y = 0; y < min_value; ++y)
    {
        for (int x = 0; x < min_value; ++x)
        {
            if (const auto depth = value.shadow.sample(uv); depth < 1.0f)
            {
                const Vec4 color = {
                    depth, depth, depth, 1.0f
                };
                viewport.put_pixel(x+offset, y, color);
            }
            uv.x += delta;
        }
        uv.x = 0.0f;
        uv.y += delta;
    }
}

void RendererRaster::render_multithread() noexcept
{
    std::array<G_buffer, Viewport::TILE_SIZE * Viewport::TILE_SIZE> g_buffer{};
    int last_seen_frame = 0;
    while (!stop_flag.load(std::memory_order_relaxed)) {
        frame_counter.wait(last_seen_frame);
        last_seen_frame = frame_counter.load(std::memory_order_relaxed);

        if (stop_flag.load(std::memory_order_relaxed)) break;

        while (true) {
            constexpr int CHUNK_SIZE = 8;
            const int t_start = tile_index.fetch_add(CHUNK_SIZE, std::memory_order_relaxed);
            if (t_start >= viewport.grid.total_tiles) break;

            const int t_end = std::min(t_start + CHUNK_SIZE, viewport.grid.total_tiles);
            for (int t = t_start; t < t_end; ++t)
            {
                const auto& tile = viewport.grid.tiles[t];
                if (tile.triangles_id.empty()) continue;
                if (render_mode == RenderMode::DEFERRED_TILED_M)
                {
                    g_buffer.fill({});
                    render_tile_deferred(tile, g_buffer);
                };
                if (render_mode == RenderMode::FORWARD_TILED_M) render_tile_forward(tile);
            }
        }

        if (active_workers.fetch_sub(1) == 1) {
            active_workers.notify_one();
        }
    }
}

void RendererRaster::draw_line(Vec3 a, Vec3 b) noexcept
{
    const Vec4 color = {0.2f,0.4f,0.2f, 1.0f};
    const auto dx = b.x - a.x;
    const auto dy = b.y - a.y;

    constexpr float depth_bias = 0.0001f;

    if (std::abs(dx) > std::abs(dy)) {
        if (dx < 0.0f) {
            std::swap(a,b);
        }

        const auto ab_y = (b.y-a.y) / (b.x-a.x);
        const auto ab_z = (b.z-a.z) / (b.x-a.x);
        auto ys = a.y;
        auto zs = a.z - depth_bias;
        for (float x = a.x; x <= b.x; ++x) {
            if (
                x > 0.0f && x < static_cast<float>(viewport.width) &&
                ys > 0.0f && ys < static_cast<float>(viewport.height) &&
                viewport.depth_pass(static_cast<int>(x), static_cast<int>(ys), zs))
            {
                viewport.put_pixel(static_cast<int>(x), static_cast<int>(ys), color);
            }
            ys += ab_y;
            zs += ab_z;
        }
        return;
    }

    if (dy < 0.0f) {
        std::swap(a,b);
    }

    const auto ab_x = (b.x-a.x) / (b.y-a.y);
    const auto ab_z = (b.z-a.z) / (b.y-a.y);
    auto xs = a.x;
    auto zs = a.z - depth_bias;

    for (float y = a.y; y <= b.y; ++y) {
        if (
                xs > 0.0f && xs < static_cast<float>(viewport.width) &&
                y > 0.0f && y < static_cast<float>(viewport.height) &&
                viewport.depth_pass(static_cast<int>(xs), static_cast<int>(y), zs))
        {
            viewport.put_pixel(static_cast<int>(xs), static_cast<int>(y), color);
        }
        xs += ab_x;
        zs += ab_z;
    }
}

void RendererRaster::draw_aabb(const AABB2D &aabb) noexcept
{
    draw_line({aabb.min.x, aabb.min.y, -1.0f}, {aabb.min.x, aabb.max.y, -1.0f});
    draw_line({aabb.min.x, aabb.max.y, -1.0f}, {aabb.max.x, aabb.max.y, -1.0f});
    draw_line({aabb.max.x, aabb.max.y, -1.0f}, {aabb.max.x, aabb.min.y, -1.0f});
    draw_line({aabb.max.x, aabb.min.y, -1.0f}, {aabb.min.x, aabb.min.y, -1.0f});
}

void RendererRaster::draw_wireframe_triangle(const FullTriangle &triangle) noexcept
{
    draw_line(triangle.screen_points[0], triangle.screen_points[1] );
    draw_line(triangle.screen_points[1], triangle.screen_points[2] );
    draw_line(triangle.screen_points[2], triangle.screen_points[0] );
}

void RendererRaster::draw_wireframe_from_tri_buffer() noexcept
{
    for (const auto &tri : tris_buffer)
    {
        draw_wireframe_triangle(tri);
    }
}

void RendererRaster::draw_triangle_aabb() noexcept
{
    for (const auto& tri : tris_buffer)
    {
        draw_aabb(tri.aabb);
    }
}

void RendererRaster::draw_active_tiles() noexcept
{
    AABB2D temp_aabb {};

    for (const auto& tile : viewport.grid.tiles)
    {
        if (tile.triangles_id.empty()) continue;
        temp_aabb.min.x = static_cast<float>(tile.offset_x);
        temp_aabb.min.y = static_cast<float>(tile.offset_y);
        temp_aabb.max.x = static_cast<float>(tile.offset_x) + Viewport::TILE_SIZE;
        temp_aabb.max.y = static_cast<float>(tile.offset_y) + Viewport::TILE_SIZE;

        draw_aabb(temp_aabb);
    }
}

std::string RendererRaster::renderer_mode() const noexcept
{
    switch (render_mode)
    {
        case RenderMode::FORWARD_TILED_M:
            return "forward - threaded";
        case RenderMode::DEFERRED_TILED_M:
            return "differed - threaded";
        case RenderMode::SHADOW_MAPPING:
            return "shadows - mode";
        case RenderMode::MAX_VALUE:
            return "Shouldn't happen!!";
    }
    return "";
}

void RendererRaster::toggle_render_depth()
{
    render_depth = !render_depth;
}

void RendererRaster::toggle_wireframe()
{
    render_wireframe = !render_wireframe;
}

void RendererRaster::toggle_render_normal()
{
    render_normal = !render_normal;
}

void RendererRaster::toggle_render_light()
{
    render_light = !render_light;
}

void RendererRaster::toggle_render_triangle_aabb()
{
    render_triangle_aabb = !render_triangle_aabb;
}

void RendererRaster::toggle_render_active_tiles()
{
    render_active_tiles = !render_active_tiles;
}

void RendererRaster::toggle_render_mode()
{
    shadow_index = 0;
    render_mode = static_cast<RenderMode>((static_cast<int>(render_mode)+1)%static_cast<int>(RenderMode::MAX_VALUE));
}

void RendererRaster::cycle_shadow_index()
{
    if (scene)
    {
        shadow_index = (shadow_index+1) % static_cast<int>(scene->lights.size());
    }
}

void RendererRaster::handle_input()
{
    if (IsKeyPressed(KEY_L)) toggle_render_light();
    if (IsKeyPressed(KEY_X)) toggle_wireframe();
    if (IsKeyPressed(KEY_Z)) toggle_render_depth();
    if (IsKeyPressed(KEY_N)) toggle_render_normal();
    if (IsKeyPressed(KEY_T)) toggle_render_triangle_aabb();
    if (IsKeyPressed(KEY_C)) toggle_render_active_tiles();
    if (IsKeyPressed(KEY_F)) toggle_render_mode();
    if (IsKeyPressed(KEY_R)) cycle_shadow_index();
}
