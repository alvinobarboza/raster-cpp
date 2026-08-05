#include "colliders/sphere.h"

void BoundingSphere::calculate_boundaries(const std::vector<Vec3> &verts, const Matrix4x4 &scale)
{
    radius = 0.0f;
    center = {};
    center_world = {};

    for (const auto &vert : verts) {
        center += vert;
    }

    center = center / static_cast<float>(verts.size());

    Vec3 scaled = {};
    float r = 0.0f;

    for (const auto &vert : verts) {
        scaled = vert * scale;
        scaled = scaled - center;

        r = scaled.length();

        if (radius < r) radius = r;
    }
}

std::ostream& operator<<(std::ostream &os, const BoundingSphere &s)
{
    os << "[ center:" << s.center << ", radius:" << s.radius << ", world:" << s.center_world << " ]";
    return os;
}