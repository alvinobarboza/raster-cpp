#pragma once

#include <array>
#include <vector>

#include "colliders/aabb.h"
#include "material/material.h"
#include "material/texture.h"
#include "transforms/vec2.h"
#include "transforms/vec3.h"

class Triangle {
public:
    int v1 {}, v2 {}, v3 {};
    int u1 {}, u2 {}, u3 {};
    int n1 {}, n2 {}, n3 {};
    int material_id { -1 };
    bool smooth {};

    [[nodiscard]] bool is_back_facing(const std::vector<Vec3> &vertices, const std::vector<Vec3> &normals) const;
};

struct alignas(16) Vertex {
    Vec3 point;
    Vec3 normal;
    Vec2 uv;
};

class FullTriangle {
public:
    std::array<Vertex, 3> vertices {};
    std::array<Vec2, 3> projected_uv {};
    std::array<Vec3, 3> screen_points {};
    std::array<float, 3> depth_z {};
    AABB2D aabb {};
    Vec3 normal {};
    Vec3 tangent {};
    bool smooth {};

    const MaterialRaster *material;


    FullTriangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        const MaterialRaster &material,
        bool smooth);

    void calculate_tri_aabb();
};

namespace triangle {
    [[nodiscard]] bool is_edge_top_or_left(const Vec3 &p1, const Vec3 &p2);
    [[nodiscard]] float edge_cross(const Vec3 &a, const Vec3 &b, const Vec3 &p);
}