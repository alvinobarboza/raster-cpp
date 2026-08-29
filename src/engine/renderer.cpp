#include "engine/renderer.h"

#include <algorithm>
#include <cmath>

#include "material/color_convertion.h"

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
        auto m_rotation = scene.camera.transform.rotation_matrix * model->transforms.rotation_matrix;
        auto m_transforms = scene.camera.transform.transformation_matrix * model->transforms.transformation_matrix;

        model->boundingSphere.center_world = model->boundingSphere.center * m_transforms;

        if (!scene.camera.frustum.is_inside_frustum(model->boundingSphere.center))
        {
            continue;
        }

        for (int i = 0; i < model->meshData.vertices.size(); i++)
        {
            model->meshData.vertices_word[i] = model->meshData.vertices[i] * m_transforms;
        }

        for (int i = 0; i < model->meshData.normals.size(); i++)
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

    std::ranges::sort(tris_buffer, [](const FullTriangle &a, const FullTriangle &b) {
        const auto a_depth = (a.depth_z[0] + a.depth_z[1] + a.depth_z[2]) / 3;
        const auto b_depth = (b.depth_z[0] + b.depth_z[1] + b.depth_z[2]) / 3;
        return a_depth > b_depth;
    });

    for (const auto& tri: tris_buffer)
    {
        render_triangle(tri, scene);
    }
};

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
    const auto edge1 = tri.projected_vertices[1].point - tri.projected_vertices[0].point;
    const auto edge2 = tri.projected_vertices[2].point - tri.projected_vertices[0].point;

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
                const float ndc_depth = tri.ndc_points[0].z * alpha + tri.ndc_points[1].z * beta + tri.ndc_points[2].z * gamma;

                if (scene.camera.depth_pass(static_cast<int>(x), static_cast<int>(y), z_depth))
                {
                    const auto uv_coord =
                            (tri.projected_vertices[0].uv * alpha +
                            tri.projected_vertices[1].uv * beta +
                            tri.projected_vertices[2].uv * gamma) / z_depth;

                    const auto fragment_coord = (tri.projected_vertices[0].point * alpha +
                            tri.projected_vertices[1].point * beta +
                            tri.projected_vertices[2].point * gamma) / z_depth;

                    auto normal = tri.normal;
                    if (tri.smooth)
                    {
                        normal =
                            ((tri.projected_vertices[0].normal * alpha +
                            tri.projected_vertices[1].normal * beta +
                            tri.projected_vertices[2].normal * gamma) / z_depth).normalized();
                    }

                    Vec4 p_color = tri.material->diffuse;
                    if (tri.material->map_diffuse)
                    {
                        p_color = tri.material->map_diffuse->texel_color(uv_coord);
                    }
                    if (scene.camera.render_depth)
                    {
                        float c = 1-ndc_depth;
                        if (c < 0.01) c = 0.01;
                        p_color.x = c;
                        p_color.y = c;
                        p_color.z = c;
                        p_color.w = 1.0f;
                    }
                    else
                    {
                        if (tri.material->map_normal)
                        {
                            // test - light implementation
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
                            p_color.x = normal.x * .5f + .5f;
                            p_color.y = normal.y * .5f + .5f;
                            p_color.z = -normal.z * .5f + .5f; // to render opengl style, as my handedness is the opposite
                        }
                        else if (render_light)
                        {
                            float shininess = 0.0f;
                            if (tri.material->map_roughness)
                            {
                                // // 2. Sample map_Ns and scale it to a realistic specular exponent range
                                // float sampledNs = texture2D(map_Ns, vUv).r;
                                // float maxShininess = 1000.0; // Standard maximum for Blinn-Phong
                                // float shininess = sampledNs * maxShininess;
                                shininess = tri.material->map_roughness->texel_intensity(uv_coord) * 1000.0f;
                            }
                            for (const auto& light : scene.lights)
                            {
                                // shader code todo: specular and need to load Ks in resource manager
                                // vec3 N = normalize(vNormal);
                                // vec3 L = normalize(vLightDir);
                                // vec3 V = normalize(vViewDir);
                                //
                                float specular_strength = 0.0f;
                                if (shininess > 0.0f)
                                {
                                    // // 3. Calculate Blinn-Phong specular intensity using Halfway Vector (H)
                                    // vec3 H = normalize(L + V);
                                    // float specAngle = max(dot(N, H), 0.0);
                                    // float specularIntensity = pow(specAngle, shininess);
                                    const auto view_dir = -fragment_coord;
                                    const auto h = (light.direction_world + view_dir).normalized();
                                    const auto spec_angle = std::max(0.0f, normal * h);
                                    specular_strength = std::pow(spec_angle, shininess);


                                    // TODO: 4. Combine with Ks data
                                    // vec3 finalSpecular = Ks * specularIntensity;
                                    //
                                    // // Combine with your ambient and diffuse colors below...
                                    // gl_FragColor = vec4(finalSpecular, 1.0);
                                }


                                const auto ambient = light.color * scene.skybox.ambient_intensity;
                                const auto light_intensity = std::max(0.0f, normal * light.direction_world);
                                const auto computed_intensity = light_intensity * light.intensity;
                                const auto light_color = light.color * computed_intensity;
                                const auto computed_color = ambient + light_color;
                                p_color.x = std::min(.95f, (computed_color.x * p_color.x) + (computed_color.x * p_color.x)*specular_strength);
                                p_color.y = std::min(.95f, (computed_color.y * p_color.y) + (computed_color.y * p_color.y)*specular_strength);
                                p_color.z = std::min(.95f, (computed_color.z * p_color.z) + (computed_color.z * p_color.z)*specular_strength);
                            }
                        }

                    }
                    scene.camera.put_pixel(static_cast<int>(x), static_cast<int>(y), p_color);
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
