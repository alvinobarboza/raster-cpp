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
    const Vec3& n, const Vec3& t,
    const bool smooth) : smooth(smooth), material(&material)
{
    depth_z[0] = 1 / v1.point.z;
    depth_z[1] = 1 / v2.point.z;
    depth_z[2] = 1 / v3.point.z;

    projected_normal[0] = v1.normal * depth_z[0];
    projected_normal[1] = v2.normal * depth_z[1];
    projected_normal[2] = v3.normal * depth_z[2];

    projected_uv[0] = v1.uv * depth_z[0];
    projected_uv[1] = v2.uv * depth_z[1];
    projected_uv[2] = v3.uv * depth_z[2];

    normal = n;
    tangent = t;
}

FullTriangle::FullTriangle(
    const float z1, const float z2, const float z3,
    const Vec3 &sc1, const Vec3 &sc2, const Vec3 &sc3,
    const Vec3 &n1, const Vec3 &n2, const Vec3 &n3,
    const Vec2 &uv1, const Vec2 &uv2, const Vec2 &uv3,
    const MaterialRaster &material,
    const Vec3& n, const Vec3& t,
    const bool smooth) : smooth(smooth), material(&material)
{
    depth_z[0] = z1;
    depth_z[1] = z2;
    depth_z[2] = z3;

    screen_points[0] = sc1;
    screen_points[1] = sc2;
    screen_points[2] = sc3;

    projected_normal[0] = n1 * depth_z[0];
    projected_normal[1] = n2 * depth_z[1];
    projected_normal[2] = n3 * depth_z[2];

    projected_uv[0] = uv1 * depth_z[0];
    projected_uv[1] = uv2 * depth_z[1];
    projected_uv[2] = uv3 * depth_z[2];

    normal = n;
    tangent = t;

    calculate_tri_aabb();
}

void FullTriangle::calculate_tri_aabb()
{
    const auto min_x {std::min(screen_points[0].x, std::min(screen_points[1].x, screen_points[2].x))};
    const auto min_y {std::min(screen_points[0].y, std::min(screen_points[1].y, screen_points[2].y))};
    const auto max_x {std::max(screen_points[0].x, std::max(screen_points[1].x, screen_points[2].x))};
    const auto max_y {std::max(screen_points[0].y, std::max(screen_points[1].y, screen_points[2].y))};

    aabb = {
        {
            static_cast<float>(static_cast<int>(min_x)),
            static_cast<float>(static_cast<int>(min_y))
        },
        {
            static_cast<float>(static_cast<int>(max_x)+1),
            static_cast<float>(static_cast<int>(max_y)+1)
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

Vec3 FullTriangle::frag_normal(
    const float alpha, const float beta, const float gamma,
    const Vec2 uv, const float depth) const noexcept
{
    const auto _normal = !smooth ? normal :
                    ((projected_normal[0] * alpha +
                    projected_normal[1] * beta +
                    projected_normal[2] * gamma) * depth).normalized();

    if (!material->map_normal) return _normal;

    const auto normal_map = material->map_normal->texel_normal(uv);
    const auto nt = _normal * tangent;
    const auto t = (tangent - (_normal * nt)).normalized();
    const auto b = t.cross(_normal);

    return (t * normal_map.x) + (b * normal_map.y) + (_normal * normal_map.z);
}

Vec3 FullTriangle::frag_normal(
    const Vec3& n, const Vec2 uv) const noexcept
{
    const auto _normal = smooth ? n.normalized() : normal;

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
        material->map_diffuse->bilinear_color_gamma(uv) : material->diffuse;
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
    const auto min_x {std::min(screen_points[0].x, std::min(screen_points[1].x, screen_points[2].x))};
    const auto min_y {std::min(screen_points[0].y, std::min(screen_points[1].y, screen_points[2].y))};
    const auto max_x {std::max(screen_points[0].x, std::max(screen_points[1].x, screen_points[2].x))};
    const auto max_y {std::max(screen_points[0].y, std::max(screen_points[1].y, screen_points[2].y))};

    aabb = {
        {
            static_cast<float>(static_cast<int>(min_x)),
            static_cast<float>(static_cast<int>(min_y))
        },
        {
            static_cast<float>(static_cast<int>(max_x)+1),
            static_cast<float>(static_cast<int>(max_y)+1)
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
