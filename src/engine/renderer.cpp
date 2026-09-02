#include "engine/renderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "engine/timer.h"
#include "material/color_convertion.h"
#include "transforms/constants.h"

void RendererRaster::clip_triangle(const Plane& near, const Plane& far)
{
    for (const std::array planes = {near, far}; auto& plane : planes) {
        std::swap(verts_in, verts_out);
        verts_out.clear();

        int prev_index = 0;
        for (int i = 0; i < verts_in.size(); i++)
        {
            prev_index = i - 1;
            if (prev_index < 0)
            {
                prev_index = static_cast<int>(verts_in.size()) - 1;
            }

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
        }
    }
}

void RendererRaster::render_scene(const SceneRaster &scene)
{
    scene.camera.clear_frame_buffer();
    tris_buffer.clear();

    //update lights
    for (auto &light: scene.lights)
    {
        // Since this is used only for the dot product between the light and triangle normal, I'm inverting here
        // Normal UP * actual light direction, will always produce negative value for a correct light setup.
        light.direction_world = -((light.direction * scene.camera.transform.rotation_matrix).normalized());
    }

    for (auto& model : scene.models) {
        Timer time{model->name};
        auto m_rotation = scene.camera.transform.rotation_matrix * model->transforms.rotation_matrix;
        auto m_transforms = scene.camera.transform.transformation_matrix * model->transforms.transformation_matrix;

        model->boundingSphere.center_world = model->boundingSphere.center * m_transforms;

        if (!scene.camera.frustum.is_inside_frustum(model->boundingSphere.center))
        {
            continue;
        }

        {
            Timer time_{"transform "+model->name};
            for (int i = 0; i < model->meshData.vertices.size(); i++)
            {
                model->meshData.vertices_word[i] = model->meshData.vertices[i] * m_transforms;
            }

            for (int i = 0; i < model->meshData.normals.size(); i++)
            {
                model->meshData.normals_word[i] = model->meshData.normals[i] * m_rotation;
            }
        }

        {
            Timer time_{"clip "+model->name};
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
                    const auto& m = model->meshData.materials[t.material_id];
                    for (int j = 1; j < verts_out.size() - 1; j++) {
                        auto tf = scene.camera.project_triangle(
                            verts_out[0],
                            verts_out[j],
                            verts_out[j+1],
                            m);

                        tf.smooth = t.smooth;
                        tris_buffer.push_back(tf);
                    }
                }
            }
        }
    }
    {
        Timer time{"Sort"};
        std::ranges::sort(tris_buffer, [](const FullTriangle &a, const FullTriangle &b) {
            const auto a_depth = (a.depth_z[0] + a.depth_z[1] + a.depth_z[2]) / 3;
            const auto b_depth = (b.depth_z[0] + b.depth_z[1] + b.depth_z[2]) / 3;
            return a_depth > b_depth;
        });
    }

    {
        Timer time{"render"};
        for (const auto& tri: tris_buffer)
        {
            render_triangle(tri, scene);
        }
    }
};

/*
 * Brian Will -> ?v=5p0e7YNONr8
 */

float distributionGGX(const float NdotH, const float roughness)
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f) ;
    denom = transforms::PI_R * denom * denom;
    return a2 / std::ranges::max(denom, 0.0000001f); // Prevent divide by zero
}

float geometrySmith(const float NdotV, const float NdotL, const float roughness)
{
    const float r = roughness + 1.0f;
    const float k = (r * r) / 8.0f;
    const float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
    const float ggx2 = NdotL / (NdotL * (1.0f - k) + k);
    return ggx1 * ggx2;
}

Vec3 fresnelSchlick(const float HdotV, const Vec3 baseReflectivity)
{
    const Vec3 inverse_reflectivity {1.0f - baseReflectivity.x, 1.0f - baseReflectivity.y, 1.0f - baseReflectivity.z};
    return baseReflectivity + inverse_reflectivity * std::pow(1.0f - HdotV, 5.0f);
}

void RendererRaster::render_triangle(const FullTriangle &tri, const SceneRaster &scene) const
{
    const auto minY = std::max(tri.aabb.min.y, 0.0f);
    const auto maxY = std::min(tri.aabb.max.y, static_cast<float>(scene.camera.height));
    const auto minX = std::max(tri.aabb.min.x, 0.0f);
    const auto maxX = std::min(tri.aabb.max.x, static_cast<float>(scene.camera.width));

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
    const auto p = Vec2(minX + 0.5f, minY + 0.5f);

    auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p) + bias_0;
    auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p) + bias_1;
    auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p) + bias_2;

    // Tangent calculations
    const auto edge1 = tri.vertices[1].point - tri.vertices[0].point;
    const auto edge2 = tri.vertices[2].point - tri.vertices[0].point;

    const auto deltaUV1 = tri.vertices[1].uv - tri.vertices[0].uv;
    const auto deltaUV2 = tri.vertices[2].uv - tri.vertices[0].uv;

    const auto f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

    const Vec3 tangent = Vec3(
        f*(deltaUV2.y*edge1.x - deltaUV1.y*edge2.x),
        f*(deltaUV2.y*edge1.y - deltaUV1.y*edge2.y),
        f*(deltaUV2.y*edge1.z - deltaUV1.y*edge2.z)
    ).normalized();
    // Tangent calculations

    for (float y = minY; y < maxY; y++)
    {
        auto w0 = w0_row;
        auto w1 = w1_row;
        auto w2 = w2_row;

        for (float x = minX; x < maxX; x++)
        {
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
            {
                const auto alpha = w0 * area;
                const auto beta = w1 * area;
                const auto gamma = w2 * area;

                const float z_depth = tri.depth_z[0] * alpha + tri.depth_z[1] * beta + tri.depth_z[2] * gamma;

                if (scene.camera.depth_pass(static_cast<int>(x), static_cast<int>(y), z_depth))
                {
                    const auto uv_coord =
                            (tri.projected_vertices[0].uv * alpha +
                            tri.projected_vertices[1].uv * beta +
                            tri.projected_vertices[2].uv * gamma) / z_depth;

                    const auto frag_coord = (tri.vertices[0].point * alpha +
                            tri.vertices[1].point * beta +
                            tri.vertices[2].point * gamma) / z_depth;

                    auto normal = !tri.smooth ? tri.normal :
                            ((tri.vertices[0].normal * alpha +
                            tri.vertices[1].normal * beta +
                            tri.vertices[2].normal * gamma) / z_depth).normalized();

                    const auto frag_color = tri.material->map_diffuse ?
                        tri.material->map_diffuse->texel_color(uv_coord) : tri.material->diffuse;

                    // Transforming wavefront's specular into roughness, not ideal, but will be for now
                    const float roughness = tri.material->map_roughness ?
                        tri.material->map_roughness->texel_intensity(uv_coord)
                        : tri.material->specular * 0.001f;

                    Vec4 final_color = frag_color;

                    if (tri.material->map_normal)
                    {
                        const auto normal_map = tri.material->map_normal->texel_normal(uv_coord);
                        const auto nt = normal * tangent;
                        const auto t = (tangent - (normal * nt)).normalized();
                        const auto b = t.cross(normal);

                        const auto _t = t * normal_map.x;
                        const auto _b = b * normal_map.y;
                        const auto _n = normal * normal_map.z;

                        normal = (_t + _b + _n).normalized();
                    }

                    if (scene.camera.render_normal)
                    {
                        final_color.x = normal.x * .5f + .5f;
                        final_color.y = normal.y * .5f + .5f;
                        final_color.z = -normal.z * .5f + .5f;
                    }
                    else if (scene.camera.render_depth)
                    {
                        const float ndc_depth =
                            tri.ndc_points[0].z * alpha +
                                tri.ndc_points[1].z * beta +
                                    tri.ndc_points[2].z * gamma;

                        float c = 1-ndc_depth;
                        if (c < 0.01) c = 0.01;
                        final_color.x = c;
                        final_color.y = c;
                        final_color.z = c;
                        final_color.w = 1.0f;
                    }
                    else if (render_light)
                    {
                        const auto V = (-frag_coord).normalized();
                        const auto N = normal;

                        const Vec3 albedo = {
                            std::pow(frag_color.x, 2.2f),
                            std::pow(frag_color.y, 2.2f),
                            std::pow(frag_color.z, 2.2f)
                        };

                        // If I add metallic property, lerp from 0.04 to albedo/diffuse using the range[0,1] of metallic
                        const Vec3 base_reflectivity {0.04};

                        Vec3 Lo {};
                        for (const auto& light : scene.lights)
                        {
                            // Also (light_pos - frag_pos) for point light
                            const Vec3 L = light.direction_world;
                            const Vec3 H = (V + L).normalized();

                            // This a direction light, no attenuation will be applied now
                            /*
                             *  float distance = length(light_pos - frag_pos);
                             *  float attenuation = 1.0 / (distance * distance);
                             *  vec3 radiance = ligth_color * attenuation;
                             */
                            const Vec3 radiance {light.color.x*light.intensity, light.color.y*light.intensity, light.color.z*light.intensity};

                            // Cook-Torrance BRDF
                            const float NdotV = std::max(N * V, 0.0000001f);
                            const float NdotL = std::max(N * L, 0.0000001f);
                            const float HdotV = std::max(H * V, 0.0f);
                            const float NdotH = std::max(N * H, 0.0f);

                            const float D = distributionGGX(NdotH, roughness);
                            const float G = geometrySmith(NdotV, NdotL, roughness);
                            const Vec3 F = fresnelSchlick(HdotV, base_reflectivity);

                            Vec3 specular = F * D * G;
                            specular = specular / (4.0f * NdotV * NdotL);

                            const Vec3 kD = Vec3{1.0f} - F;

                            // When using mettalic property
                            // kD *= 1.0 - mettalic;

                            // Lo += (kD * albedo / PI + specular) * radiance * NdotL;
                            const Vec3 kD_x_albedo {kD.x * albedo.x, kD.y * albedo.y, kD.z * albedo.z};
                            const Vec3 divided_pi_specular = (kD_x_albedo / transforms::PI_R + specular);
                            const Vec3 mul_radiance {
                                divided_pi_specular.x * radiance.x,
                                divided_pi_specular.y * radiance.y,
                                divided_pi_specular.z * radiance.z};

                            Lo += mul_radiance * NdotL;
                        }

                        const Vec3 ambient {
                            albedo.x * scene.skybox.ambient_intensity,
                            albedo.y * scene.skybox.ambient_intensity,
                            albedo.z * scene.skybox.ambient_intensity};
                        Vec3 color = ambient + Lo;

                        // HDR tonemapping
                        const float lum = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
                        color = color / (1.0f + lum);
                        // Gamma
                        constexpr float gamma_const {1.0f/2.2f};

                        color = {
                            std::pow(color.x, gamma_const),
                            std::pow(color.y, gamma_const),
                            std::pow(color.z, gamma_const)
                        };

                        final_color = {
                            std::clamp(color.x, 0.0f, 1.0f),
                            std::clamp(color.y, 0.0f, 1.0f),
                            std::clamp(color.z, 0.0f, 1.0f),
                            1.0f
                        };
                    }

                    scene.camera.put_pixel(static_cast<int>(x), static_cast<int>(y), final_color);
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

void RendererRaster::handle_input()
{
    if (IsKeyPressed(KEY_L)) render_light = !render_light;
}
