#include "engine/renderer.h"

#include "material/color_convertion.h"

void RendererRaster::render_scene(const SceneRaster &scene) {
    scene.camera.clear_frame_buffer();

    const auto color = color_convertion::color_to_vec4(GRAY);
    const MaterialRaster m = {"default", {color.x, color.y, color.z} , 250.0f, nullptr, nullptr, nullptr };
    for (auto& model : scene.models) {

        auto m_rotation = scene.camera.transform.rotation_matrix * model->transforms.rotation_matrix;
        auto m_transforms = scene.camera.transform.transformation_matrix * model->transforms.transformation_matrix;

        model->boundingSphere.center_world = model->boundingSphere.center * m_transforms;

        if (!scene.camera.frustum.is_inside_frustum(model->boundingSphere.center))
        {
            continue;
        }

        for (int i = 0; i < model->meshData->vertices.size(); i++)
        {
            model->meshData->vertices_word[i] = model->meshData->vertices[i] * m_transforms;
        }

        for (int i = 0; i < model->meshData->normals.size(); i++)
        {
            model->meshData->normals_word[i] = model->meshData->normals[i] * m_rotation;
        }

        for (int i = 0; i < model->meshData->triangles.size(); i++)
        {
            const auto& t = model->meshData->triangles[i];
            if (!t.is_back_facing(model->meshData->vertices_word, model->meshData->normals_word))
            {
                continue;
            }

            // TODO: fix frustum and clip triangles
            // if (
            //     !scene.camera.frustum.is_inside_frustum(model->meshData->vertices_word[t.v1]) &&
            //     !scene.camera.frustum.is_inside_frustum(model->meshData->vertices_word[t.v2]) &&
            //     !scene.camera.frustum.is_inside_frustum(model->meshData->vertices_word[t.v3]))
            // {
            //     continue;
            // }

            const auto& v1 = Vertex(model->meshData->vertices_word[t.v1],
                model->meshData->normals_word[t.n1], model->meshData->uvs[t.u1]);
            const auto& v2 = Vertex(model->meshData->vertices_word[t.v2],
                model->meshData->normals_word[t.n2], model->meshData->uvs[t.u2]);
            const auto& v3 = Vertex(model->meshData->vertices_word[t.v3],
                model->meshData->normals_word[t.n3], model->meshData->uvs[t.u3]);

            const auto tri = scene.camera.project_triangle(v1, v2, v3, m);

            // TODO: fix frustum
            bool is_inside_frustum = true;
            for (const auto & ndc : tri.ndc_points) {
                if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f || ndc.z < 0.001f || ndc.z > 1.0f) {
                    is_inside_frustum = false;
                    break;
                }
            }
            if (is_inside_frustum) {
                render_triangle(tri, scene);
            }
        }
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
        bias_0 = -0.001;
    }
    if (triangle::is_edge_top_or_left(tri.screen_points[2], tri.screen_points[0]))
    {
        bias_1 = -0.001;
    }
    if (triangle::is_edge_top_or_left(tri.screen_points[0], tri.screen_points[1]))
    {
        bias_2 = -0.001;
    }

    const auto area = 1.0f / triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], tri.screen_points[2]);
    const auto p = Vec2(minX + 0.5f, minY + 0.5f);

    auto w0_row = triangle::edge_cross(tri.screen_points[1], tri.screen_points[2], p) + bias_0;
    auto w1_row = triangle::edge_cross(tri.screen_points[2], tri.screen_points[0], p) + bias_1;
    auto w2_row = triangle::edge_cross(tri.screen_points[0], tri.screen_points[1], p) + bias_2;

    for (float y = minY; y <= maxY; y++)
    {
        auto w0 = w0_row;
        auto w1 = w1_row;
        auto w2 = w2_row;

        for (float x = minX; x <= maxX; x++)
        {
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
            {
                const auto alpha = w0 * area;
                const auto beta = w1 * area;
                const auto gamma = w2 * area;

                if (const auto depth = tri.ndc_points[0].z * alpha + tri.ndc_points[1].z * beta + tri.ndc_points[2].z * gamma;
                    scene.camera.depth_pass(static_cast<int>(x), static_cast<int>(y), depth))
                {
                    scene.camera.put_pixel(static_cast<int>(x), static_cast<int>(y), {alpha,beta,gamma,1.0f},depth);
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
