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

struct Vertex {
    Vec3 point;
    Vec3 normal;
    Vec2 uv;
};

class FullTriangle {
public:
    std::array<Vertex, 3> vertices {};
    std::array<Vertex, 3> projected_vertices {};
    std::array<Vec3, 3> ndc_points {};
    std::array<Vec2, 3> screen_points {};
    std::array<float, 3> depth_z {};
    Vec3 origin {};
    Vec3 normal {};

    AABB2D aabb {};

    const MaterialRaster *material;

    bool smooth {};

    FullTriangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        const MaterialRaster &material);

    void calculate_tri_aabb();
};

namespace triangle {
    [[nodiscard]] bool is_edge_top_or_left(const Vec2 &p1, const Vec2 &p2);
    [[nodiscard]] float edge_cross(const Vec2 &a, const Vec2 &b, const Vec2 &p);
}