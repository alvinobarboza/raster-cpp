#include "model/triangle.h"

#include <cmath>
#include <utility>

bool Triangle::is_back_facing(const std::vector<Vec3> &vertices, const std::vector<Vec3> &normals) const
{
    const float angle_a = normals[n1] * -vertices[v1];
    const float angle_b = normals[n2] * -vertices[v2];
    const float angle_c = normals[n3] * -vertices[v3];

    return angle_a >= 0 || angle_b >= 0 || angle_c >= 0;
}

FullTriangle::FullTriangle(
    const Vertex &v1,
    const Vertex &v2,
    const Vertex &v3,
    const MaterialRaster &material,
    const bool smooth) : smooth(smooth), material(&material)
{
    vertices[0] = v1;
    vertices[1] = v2;
    vertices[2] = v3;

    depth_z[0] = 1 / v1.point.z;
    depth_z[1] = 1 / v2.point.z;
    depth_z[2] = 1 / v3.point.z;

    projected_uv[0] = v1.uv * depth_z[0];
    projected_uv[1] = v2.uv * depth_z[1];
    projected_uv[2] = v3.uv * depth_z[2];

    const auto ba = v2.point - v1.point;
    const auto ca = v3.point - v1.point;

    normal = ba.cross(ca).normalized();

    const auto edge1 = vertices[1].point - vertices[0].point;
    const auto edge2 = vertices[2].point - vertices[0].point;

    const auto deltaUV1 = vertices[1].uv - vertices[0].uv;
    const auto deltaUV2 = vertices[2].uv - vertices[0].uv;

    const auto f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

    tangent = Vec3(
        f*(deltaUV2.y*edge1.x - deltaUV1.y*edge2.x),
        f*(deltaUV2.y*edge1.y - deltaUV1.y*edge2.y),
        f*(deltaUV2.y*edge1.z - deltaUV1.y*edge2.z)
    ).normalized();
}

void FullTriangle::calculate_tri_aabb()
{
    aabb = {
        {
            std::floor(
                std::min(screen_points[0].x, std::min(screen_points[1].x, screen_points[2].x))
            ),
            std::floor(
                std::min(screen_points[0].y, std::min(screen_points[1].y, screen_points[2].y))
            )
        },
        {
            std::ceil(
                std::max(screen_points[0].x, std::max(screen_points[1].x, screen_points[2].x))
            ),
            std::ceil(
                std::max(screen_points[0].y, std::max(screen_points[1].y, screen_points[2].y))
            )
        }
    };
}

float FullTriangle::frag_depth_ndc(const float alpha, const float beta, const float gamma) const noexcept
{
    return screen_points[0].z * alpha + screen_points[1].z * beta + screen_points[2].z * gamma;
}

float FullTriangle::frag_depth(const float alpha, const float beta, const float gamma) const noexcept
{
    return depth_z[0] * alpha + depth_z[1] * beta + depth_z[2] * gamma;
}

Vec2 FullTriangle::frag_uv_coord(const float alpha, const float beta, const float gamma, const float depth) const noexcept
{
    return (projected_uv[0] * alpha +
            projected_uv[1] * beta +
            projected_uv[2] * gamma) * depth;
}

Vec3 FullTriangle::frag_coord(const float alpha, const float beta, const float gamma, [[maybe_unused]]const float depth) const noexcept
{
    return (vertices[0].point * alpha +
            vertices[1].point * beta +
            vertices[2].point * gamma) ;
}

Vec3 FullTriangle::frag_normal(
    const float alpha, const float beta, const float gamma,
    const Vec2 uv, const float depth) const noexcept
{
    const auto _normal = !smooth ? normal :
                    ((vertices[0].normal * alpha +
                    vertices[1].normal * beta +
                    vertices[2].normal * gamma) * depth).normalized();

    if (!material->map_normal) return _normal;

    const auto normal_map = material->map_normal->texel_normal(uv);
    const auto nt = _normal * tangent;
    const auto t = (tangent - (_normal * nt)).normalized();
    const auto b = t.cross(_normal);

    return (t * normal_map.x) + (b * normal_map.y) + (_normal * normal_map.z);
}

Vec4 FullTriangle::frag_color(const Vec2 uv) const noexcept
{
    return material->map_diffuse ?
        material->map_diffuse->bilinear_color(uv) : material->diffuse;
}


float FullTriangle::frag_roughness(const Vec2 uv) const noexcept
{
    // Transforming wavefront's specular into roughness, not ideal, but will be for now
    return material->map_roughness ?
            material->map_roughness->texel_intensity(uv)
            : material->specular * 0.001f;
}



void ShadowTriangle::calculate_tri_aabb()
{
    aabb = {
        {
            std::floor(
                std::min(screen_points[0].x, std::min(screen_points[1].x, screen_points[2].x))
            ),
            std::floor(
                std::min(screen_points[0].y, std::min(screen_points[1].y, screen_points[2].y))
            )
        },
        {
            std::ceil(
                std::max(screen_points[0].x, std::max(screen_points[1].x, screen_points[2].x))
            ),
            std::ceil(
                std::max(screen_points[0].y, std::max(screen_points[1].y, screen_points[2].y))
            )
        }
    };
}

float ShadowTriangle::frag_depth_ndc(const float alpha, const float beta, const float gamma) const noexcept
{
    return screen_points[0].z * alpha + screen_points[1].z * beta + screen_points[2].z * gamma;
}

bool triangle::is_edge_top_or_left(const Vec3 &p1, const Vec3 &p2)
{
    const float x = p2.x - p1.x;
    const float y = p2.y - p1.y;

    const bool is_top_edge = y == 0 && x > 0;
    const bool is_left_edge = y < 0;

    return is_top_edge || is_left_edge;
}

float triangle::edge_cross(const Vec3 &a, const Vec3 &b, const Vec3 &p)
{
    const float ab_x = b.x - a.x;
    const float ab_y = b.y - a.y;

    const float ap_x = p.x - a.x;
    const float ap_y = p.y - a.y;

    return (ab_x * ap_y) - (ab_y * ap_x);
}
