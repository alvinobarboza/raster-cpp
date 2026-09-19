#include "engine/renderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "engine/timer.h"
#include "material/color_convertion.h"
#include "transforms/constants.h"

RendererRaster::RendererRaster(const int w, const int h, const int res_factor) noexcept:
viewport(w, h, res_factor) {}

void RendererRaster::clip_triangle(const Plane& near, const Plane& far) noexcept
{
    for (const std::array planes = {near, far}; auto& plane : planes) {
        std::swap(verts_in, verts_out);
        verts_out.clear();

        size_t prev_index = verts_in.size() - 1;
        for (size_t i = 0; i < verts_in.size(); i++)
        {
            const auto current_point = verts_in[i];
            const auto [point, normal, uv] = verts_in[prev_index];

            const auto distance_current_point = plane.signed_distance_to_point(current_point.point);
            const auto distance_prev_point = plane.signed_distance_to_point(point);

            if (distance_current_point > 0.0f)
            {
                if (distance_prev_point <= 0.0f)
                {
                    const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
                    verts_out.emplace_back(
                        current_point.point.lerp_to(point, ratio),
                        current_point.normal.lerp_to(normal, ratio),
                        current_point.uv.lerp_to(uv, ratio));
                }
                verts_out.push_back(current_point);
            }
            else if (distance_prev_point > 0)
            {
                const auto ratio = distance_current_point / (distance_current_point - distance_prev_point);
                verts_out.emplace_back(
                    current_point.point.lerp_to(point, ratio),
                    current_point.normal.lerp_to(normal, ratio),
                    current_point.uv.lerp_to(uv, ratio));
            }
            prev_index = i;
        }
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

void RendererRaster::render_scene(SceneRaster &scene)
{
    viewport.clear_frame_buffer();
    viewport.reset_tiles();
    tris_buffer.clear();

    //update lights
    for (auto &light: scene.lights)
    {
        // Since this is used only for the dot product between the light and triangle normal, I'm inverting here
        // Normal UP * actual light direction, will always produce negative value for a correct light setup.
        light.direction_world = -(light.direction * scene.camera.transform.rotation_matrix).normalized();
    }

    for (const auto& model: scene.models)
    {
        auto m_transforms = scene.camera.transform.transformation_matrix * model->transforms.transformation_matrix;
        model->boundingSphere.center_world = model->boundingSphere.center * m_transforms;
        model->to_render = scene.camera.frustum.is_inside_frustum(model->boundingSphere);
    }


    std::ranges::sort(scene.models, []( ModelRaster*& a, ModelRaster*& b) {
        return a->boundingSphere.center_world.length() > b->boundingSphere.center_world.length();
    });


    //int count_skipped_tris = 0;
    for (const auto& model : scene.models)
    {
        auto m_rotation = scene.camera.transform.rotation_matrix * model->transforms.rotation_matrix;
        auto m_transforms = scene.camera.transform.transformation_matrix * model->transforms.transformation_matrix;

        if (!model->to_render)
        {
            //std::cout << "[SKIP] " << model->name << "\n";
            continue;
        }

        for (size_t i = 0; i < model->meshData.vertices.size(); ++i)
        {
            model->meshData.vertices_word[i] = model->meshData.vertices[i] * m_transforms;
        }

        for (size_t i = 0; i < model->meshData.normals.size(); ++i)
        {
            model->meshData.normals_word[i] = model->meshData.normals[i] * m_rotation;
        }


        for (const auto &t: model->meshData.triangles)
        {
            if (!t.is_back_facing(model->meshData.vertices_word, model->meshData.normals_word))
            {
                continue;
            }

            const auto& v1 = Vertex(model->meshData.vertices_word[t.v1],
                model->meshData.normals_word[t.n1], model->meshData.uvs[t.u1]);
            const auto& v2 = Vertex(model->meshData.vertices_word[t.v2],
                model->meshData.normals_word[t.n2], model->meshData.uvs[t.u2]);
            const auto& v3 = Vertex(model->meshData.vertices_word[t.v3],
                model->meshData.normals_word[t.n3], model->meshData.uvs[t.u3]);

            verts_out.clear();
            verts_in.clear();

            verts_out.push_back(v1);
            verts_out.push_back(v2);
            verts_out.push_back(v3);

            clip_triangle(
            scene.camera.frustum.planes[NEAR_PLANE],
            scene.camera.frustum.planes[FAR_PLANE]);


            if (verts_out.size() > 2) {
                for (size_t j = 1; j < verts_out.size() - 1; ++j) {
                    const auto p1 = verts_out[0];
                    const auto p2 = verts_out[j];
                    const auto p3 = verts_out[j + 1];

                    FullTriangle tf {
                        p1,p2,p3,
                        model->meshData.materials[t.material_id],
                        t.smooth
                    };

                    const auto ndc0 = scene.camera.vertex_to_ndc(p1.point);
                    const auto ndc1 = scene.camera.vertex_to_ndc(p2.point);
                    const auto ndc2 = scene.camera.vertex_to_ndc(p3.point);

                    if (is_outside_screen(ndc0, ndc1, ndc2))
                    {
                        //++count_skipped_tris;
                        continue;
                    }

                    tf.screen_points[0] = viewport.ndc_to_screen(ndc0);
                    tf.screen_points[1] = viewport.ndc_to_screen(ndc1);
                    tf.screen_points[2] = viewport.ndc_to_screen(ndc2);

                    tf.calculate_tri_aabb();

                    tris_buffer.push_back(tf);
                }
            }
        }
    }
    //std::cout << "[SKIP] triangles: "<< count_skipped_tris <<"\n";

    if (render_forward)
    {
        Timer time{"render-forward"};
        for (const auto& tri: tris_buffer)
        {
            render_triangle(tri, scene);
        }
    }

    if (render_deferred)
    {
        Timer time{"render-deferred"};
        viewport.bin_triangles(tris_buffer);
        render_tiles(scene);
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

/*
 * Brian Will -> ?v=5p0e7YNONr8
 */

float distributionGGX(const float NdotH, const float roughness)
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f ;
    denom = transforms::PI_R * denom * denom;
    return a2 / std::ranges::max(denom, 0.0000001f); // Prevent divide by zero
}

float geometrySmith(const float NdotV, const float NdotL, const float roughness)
{
    const float r = roughness + 1.0f;
    const float k = r * r / 8.0f;
    const float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
    const float ggx2 = NdotL / (NdotL * (1.0f - k) + k);
    return ggx1 * ggx2;
}

Vec3 fresnelSchlick(const float HdotV, const Vec3 baseReflectivity)
{
    const Vec3 inverse_reflectivity {1.0f - baseReflectivity.x, 1.0f - baseReflectivity.y, 1.0f - baseReflectivity.z};
    return baseReflectivity + inverse_reflectivity * std::pow(1.0f - HdotV, 5.0f);
}

Vec4 calculate_light(
    const std::vector<Light>& lights,
    const float frag_rough, const Vec4 frag_color,const Vec3 frag_normal,
    const Vec3 view_normal, const float ambient_intensity) noexcept
{
    const Vec3 albedo = {
        std::pow(frag_color.x, 2.2f),
        std::pow(frag_color.y, 2.2f),
        std::pow(frag_color.z, 2.2f)
    };

    // If I add metallic property, lerp from 0.04 to albedo/diffuse using the range[0,1] of metallic
    const Vec3 base_reflectivity {0.04};

    Vec3 Lo {};
    for (const auto& light : lights)
    {
        // Also (light_pos - frag_pos) for point light
        const Vec3 L = light.direction_world;
        const Vec3 H = (view_normal + L).normalized();

        // This a direction light, no attenuation will be applied now
        /*
         *  float distance = length(light_pos - frag_pos);
         *  float attenuation = 1.0 / (distance * distance);
         *  vec3 radiance = ligth_color * attenuation;
         */
        const Vec3 radiance {light.color.x*light.intensity, light.color.y*light.intensity, light.color.z*light.intensity};

        // Cook-Torrance BRDF
        const float NdotV = std::max(frag_normal * view_normal, 0.0000001f);
        const float NdotL = std::max(frag_normal * L, 0.0000001f);
        const float HdotV = std::max(H * view_normal, 0.0f);
        const float NdotH = std::max(frag_normal * H, 0.0f);

        const float D = distributionGGX(NdotH, frag_rough);
        const float G = geometrySmith(NdotV, NdotL, frag_rough);
        const Vec3 F = fresnelSchlick(HdotV, base_reflectivity);

        const Vec3 specular = F * D * G / (4.0f * NdotV * NdotL);

        const Vec3 kD = Vec3{1.0f} - F;

        // When using metallic property
        // kD *= 1.0 - metallic;

        // Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        const Vec3 kD_x_albedo {kD.x * albedo.x, kD.y * albedo.y, kD.z * albedo.z};
        const Vec3 divided_pi_specular = kD_x_albedo / transforms::PI_R + specular;
        const Vec3 mul_radiance {
            divided_pi_specular.x * radiance.x,
            divided_pi_specular.y * radiance.y,
            divided_pi_specular.z * radiance.z};

        Lo += mul_radiance * NdotL;
    }

    const Vec3 ambient {albedo * ambient_intensity};
    Vec3 color = ambient + Lo;

    // HDR tone mapping
    const float lum = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
    color = color / (1.0f + lum);
    // Gamma
    constexpr float gamma_const {1.0f/2.2f};

    color = {
        std::pow(color.x, gamma_const),
        std::pow(color.y, gamma_const),
        std::pow(color.z, gamma_const)
    };

    return {
        std::clamp(color.x, 0.0f, 1.0f),
        std::clamp(color.y, 0.0f, 1.0f),
        std::clamp(color.z, 0.0f, 1.0f),
        1.0f
    };
}

void RendererRaster::render_tiles(const SceneRaster &scene) noexcept
{
    std::array<Gbuffer, Viewport::TILE_SIZE * Viewport::TILE_SIZE> g_buffer{};
    for (int g_y = 0; g_y < viewport.grid.height; ++g_y)
    {
        const int offset_y = g_y * Viewport::TILE_SIZE;
        for (int g_x = 0; g_x < viewport.grid.width; ++g_x)
        {
            const int offset_x = g_x * Viewport::TILE_SIZE;
            const auto tile_i = g_y * viewport.grid.width + g_x;

            if (const auto& tile = viewport.grid.tiles[tile_i]; tile.counter > 0)
            {
                g_buffer.fill({});

                for (int i = tile.offset; i < tile.offset + tile.counter; ++i)
                {
                    const int triangle_id = viewport.grid.triangles_id[i];
                    const auto& tri = tris_buffer[triangle_id];
                    const auto delta_w0_col = tri.screen_points[1].y - tri.screen_points[2].y;
                    const auto delta_w1_col = tri.screen_points[2].y - tri.screen_points[0].y;
                    const auto delta_w2_col = tri.screen_points[0].y - tri.screen_points[1].y;

                    const auto delta_w0_row = tri.screen_points[2].x - tri.screen_points[1].x;
                    const auto delta_w1_row = tri.screen_points[0].x - tri.screen_points[2].x;
                    const auto delta_w2_row = tri.screen_points[1].x - tri.screen_points[0].x;

                    float bias_0 = 0.0f, bias_1 = 0.0f, bias_2 = 0.0f;
                    if (triangle::is_edge_top_or_left(tri.screen_points[1], tri.screen_points[2]))
                    {
                        bias_0 = -0.0001;
                    }
                    if (triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0]))
                    {
                        bias_1 = -0.0001;
                    }
                    if (triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1]))
                    {
                        bias_2 = -0.0001;
                    }

                    const auto cross = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
                    if (cross < 1e-6f) continue; // possible edge case
                    const auto area = 1.0f / cross;
                    const Vec3 p = {static_cast<float>(offset_x) + 0.5f, static_cast<float>(offset_y) + 0.5f, 0.0f};

                    auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p) + bias_0;
                    auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p) + bias_1;
                    auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p) + bias_2;

                    for (int y = 0; y < Viewport::TILE_SIZE; y++)
                    {
                        auto w0 = w0_row;
                        auto w1 = w1_row;
                        auto w2 = w2_row;

                        for (int x = 0; x < Viewport::TILE_SIZE; x++)
                        {
                            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
                            {
                                const auto alpha = w0 * area;
                                const auto beta = w1 * area;
                                const auto gamma = w2 * area;

                                const int index = x + y * Viewport::TILE_SIZE;
                                if (const float z_depth = tri.frag_depth_ndc(alpha, beta, gamma);
                                    g_buffer[index].depth > z_depth)
                                {
                                    g_buffer[index].depth = z_depth;

                                    const auto frag_depth = 1 / tri.frag_depth(alpha, beta, gamma);
                                    const auto frag_uv = tri.frag_uv_coord(alpha, beta, gamma, frag_depth);
                                    const auto frag_coord = tri.frag_coord(alpha, beta, gamma, frag_depth);
                                    const auto frag_normal = tri.frag_normal(alpha, beta, gamma, frag_uv,frag_depth);
                                    const auto frag_color = tri.frag_color(frag_uv);
                                    const float frag_rough = tri.frag_roughness(frag_uv);

                                    g_buffer[index].frag_coord = frag_coord;
                                    g_buffer[index].albedo = {frag_color.x, frag_color.y, frag_color.z};
                                    g_buffer[index].normal = frag_normal;
                                    g_buffer[index].roughness = frag_rough;
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


                for (int y = 0; y < Viewport::TILE_SIZE; y++)
                {
                    for (int x = 0; x < Viewport::TILE_SIZE; x++)
                    {
                        const int index = x + y * Viewport::TILE_SIZE;
                        const auto& gb = g_buffer[index];
                        if (gb.depth > 2.0f) continue;

                        const auto px = offset_x + x;
                        const auto py = offset_y + y;

                        if (px >= viewport.width || py >= viewport.height) continue;

                        if (render_normal)
                        {
                            const Vec4 final_color{
                                gb.normal.x * .5f + .5f,
                                gb.normal.y * .5f + .5f,
                                -gb.normal.z * .5f + .5f,
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
                            const Vec4 final_color{gb.albedo.x, gb.albedo.y, gb.albedo.z, 1.0f};
                            viewport.put_pixel(px, py, final_color);
                            continue;
                        }

                        const auto view_normal = (-gb.frag_coord).normalized();
                        const Vec4 albedo{gb.albedo.x, gb.albedo.y, gb.albedo.z, 1.0f};
                        const auto final_color = calculate_light(
                            scene.lights, gb.roughness, albedo,
                            gb.normal, view_normal, scene.skybox.ambient_intensity);

                        viewport.put_pixel(px, py, final_color);
                    }
                }
            }
        }
    }
}

void RendererRaster::render_triangle(const FullTriangle &tri, const SceneRaster &scene) noexcept
{
    const auto minY = std::max(tri.aabb.min.y, 0.0f);
    const auto maxY = std::min(tri.aabb.max.y, static_cast<float>(viewport.height));
    const auto minX = std::max(tri.aabb.min.x, 0.0f);
    const auto maxX = std::min(tri.aabb.max.x, static_cast<float>(viewport.width));

    const auto delta_w0_col = tri.screen_points[1].y - tri.screen_points[2].y;
    const auto delta_w1_col = tri.screen_points[2].y - tri.screen_points[0].y;
    const auto delta_w2_col = tri.screen_points[0].y - tri.screen_points[1].y;

    const auto delta_w0_row = tri.screen_points[2].x - tri.screen_points[1].x;
    const auto delta_w1_row = tri.screen_points[0].x - tri.screen_points[2].x;
    const auto delta_w2_row = tri.screen_points[1].x - tri.screen_points[0].x;

    float bias_0 = 0.0f, bias_1 = 0.0f, bias_2 = 0.0f;
    if (triangle::is_edge_top_or_left(tri.screen_points[1], tri.screen_points[2]))
    {
        bias_0 = -0.0001;
    }
    if (triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0]))
    {
        bias_1 = -0.0001;
    }
    if (triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1]))
    {
        bias_2 = -0.0001;
    }

    const auto area = 1.0f / triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
    const Vec3 p = {minX + 0.5f, minY + 0.5f, 0.0f};

    auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p) + bias_0;
    auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p) + bias_1;
    auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p) + bias_2;

    for (int y = minY; y < maxY; y++)
    {
        auto w0 = w0_row;
        auto w1 = w1_row;
        auto w2 = w2_row;

        for (int x = minX; x < maxX; x++)
        {
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
            {
                const auto alpha = w0 * area;
                const auto beta = w1 * area;
                const auto gamma = w2 * area;

                if (const float z_depth = tri.frag_depth(alpha, beta, gamma);
                    viewport.depth_pass(x, y, z_depth))
                {
                    const auto frag_depth = 1 / z_depth;
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
                        const float ndc_depth =
                            tri.screen_points[0].z * alpha +
                                tri.screen_points[1].z * beta +
                                    tri.screen_points[2].z * gamma;

                        float c = 1-ndc_depth;
                        if (c < 0.01) c = 0.01;

                        const Vec4 final_color {
                            c,c,c,1.0f
                        };

                        viewport.put_pixel(x, y, final_color);
                    }
                    else if (render_light)
                    {
                        const auto view_normal = (-frag_coord).normalized();
                        const auto final_color = calculate_light(
                            scene.lights, frag_rough, frag_color, frag_normal,
                            view_normal, scene.skybox.ambient_intensity);

                        viewport.put_pixel(x, y, final_color);
                    }
                    else
                    {
                        viewport.put_pixel(x, y, frag_color);
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

void RendererRaster::draw_line(Vec3 a, Vec3 b) noexcept
{
    const Vec4 color = {0.4f,0.2f,0.2f, 1.0f};
    const auto dx = b.x - a.x;
    const auto dy = b.y - a.y;

    if (std::abs(dx) > std::abs(dy)) {
        if (dx < 0.0f) {
            std::swap(a,b);
        }

        const auto ab_y = (b.y-a.y) / (b.x-a.x);
        auto ys = a.y;
        for (float x = a.x; x <= b.x; ++x) {
            if (
                x > 0.0f && x < static_cast<float>(viewport.width) &&
                ys > 0.0f && ys < static_cast<float>(viewport.height))
            {
                viewport.put_pixel(static_cast<int>(x), static_cast<int>(ys), color);
            }
            ys += ab_y;
        }
        return;
    }

    if (dy < 0.0f) {
        std::swap(a,b);
    }

    const auto ab_x = (b.x-a.x) / (b.y-a.y);
    auto xs = a.x;

    for (float y = a.y; y <= b.y; ++y) {
        if (
                xs > 0.0f && xs < static_cast<float>(viewport.width) &&
                y > 0.0f && y < static_cast<float>(viewport.height))
        {
            viewport.put_pixel(static_cast<int>(xs), static_cast<int>(y), color);
        }
        xs += ab_x;
    }
}

void RendererRaster::draw_aabb(const AABB2D &aabb) noexcept
{
    draw_line({aabb.min.x, aabb.min.y, 1.0f}, {aabb.min.x, aabb.max.y, 1.0f});
    draw_line({aabb.min.x, aabb.max.y, 1.0f}, {aabb.max.x, aabb.max.y, 1.0f});
    draw_line({aabb.max.x, aabb.max.y, 1.0f}, {aabb.max.x, aabb.min.y, 1.0f});
    draw_line({aabb.max.x, aabb.min.y, 1.0f}, {aabb.min.x, aabb.min.y, 1.0f});
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
    int offset_x { 0 };
    int offset_y { 0 };
    AABB2D temp_aabb {};
    for (int g_y = 0; g_y < viewport.grid.height; ++g_y)
    {
        for (int g_x = 0; g_x < viewport.grid.width; ++g_x)
        {
            const auto tile_i = g_y * viewport.grid.width + g_x;

            if (const auto& tile = viewport.grid.tiles[tile_i]; tile.counter > 0)
            {
                temp_aabb.min.x = static_cast<float>(offset_x);
                temp_aabb.min.y = static_cast<float>(offset_y);
                temp_aabb.max.x = static_cast<float>(offset_x) + Viewport::TILE_SIZE;
                temp_aabb.max.y = static_cast<float>(offset_y) + Viewport::TILE_SIZE;

                draw_aabb(temp_aabb);
            }

            offset_x += Viewport::TILE_SIZE;
        }
        offset_x = 0;
        offset_y += Viewport::TILE_SIZE;
    }
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

void RendererRaster::toggle_render_forward()
{
    render_forward = !render_forward;
    render_deferred = !render_forward;
}

void RendererRaster::toggle_render_deferred()
{
    render_deferred = !render_deferred;
    render_forward = !render_deferred;
}

void RendererRaster::handle_input()
{
    if (IsKeyPressed(KEY_L)) toggle_render_light();
    if (IsKeyPressed(KEY_X)) toggle_wireframe();
    if (IsKeyPressed(KEY_Z)) toggle_render_depth();
    if (IsKeyPressed(KEY_N)) toggle_render_normal();
    if (IsKeyPressed(KEY_T)) toggle_render_triangle_aabb();
    if (IsKeyPressed(KEY_C)) toggle_render_active_tiles();
    if (IsKeyPressed(KEY_F)) toggle_render_forward();
    if (IsKeyPressed(KEY_R)) toggle_render_deferred();
}
