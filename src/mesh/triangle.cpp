#include "mesh/triangle.h"

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
    TextureRaster *albedo,
    TextureRaster *normal,
    TextureRaster *specular) : albedo(albedo), normal(normal), specular(specular)
{
    vertices[0] = v1;
    vertices[1] = v2;
    vertices[2] = v3;
}

bool triangle::is_edge_top_or_left(const Vec2 &p1, const Vec2 &p2)
{
    const float x = p2.x - p1.x;
    const float y = p2.y - p1.y;

    const bool is_top_edge = y == 0 && x > 0;
    const bool is_left_edge = y < 0;

    return is_top_edge || is_left_edge;
}

float triangle::edge_cross(const Vec2 &a, const Vec2 &b, const Vec2 &p)
{
    const float ab_x = b.x - a.x;
    const float ab_y = b.y - a.y;

    const float ap_x = p.x - a.x;
    const float ap_y = p.y - a.y;

    return (ab_x * ap_y) - (ab_y * ap_x);
}
