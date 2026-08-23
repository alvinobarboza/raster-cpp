#include "engine/renderer.h"

#include <algorithm>

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

        for (int i = 0; i < model->meshData.triangles.size(); i++)
        {
            const auto& t = model->meshData.triangles[i];
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

void RendererRaster::render_triangle(const FullTriangle &tri, const SceneRaster &scene)
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
                    const Vec3 tmp_color = tri.material->diffuse;
                    Vec4 p_color = {tmp_color.x, tmp_color.y, tmp_color.z, 1.0f};
                    if (tri.material->map_diffuse)
                    {
                        const auto uv_coord =
                            tri.projected_vertices[0].uv * alpha +
                            tri.projected_vertices[1].uv * beta +
                            tri.projected_vertices[2].uv * gamma;
                        p_color = tri.material->map_diffuse->texel_color(uv_coord / z_depth);
                    }
                    scene.camera.put_pixel(static_cast<int>(x), static_cast<int>(y), p_color, ndc_depth);
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
