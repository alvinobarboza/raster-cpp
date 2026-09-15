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
