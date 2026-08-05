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
    int v1 = 0, v2 = 0, v3 = 0;
    int u1 = 0, u2 = 0, u3 = 0;
    int n1 = 0, n2 = 0, n3 = 0;
    int material_id = -1;
    bool smooth = false;

    [[nodiscard]] bool is_back_facing(const std::vector<vec3> &vertices, const std::vector<vec3> &normals) const;
};

struct Vertex {
    vec3 point;
    vec3 normal;
    vec2 uv;
};

class FullTriangle {
public:
    std::array<Vertex, 3> vertices = {};
    std::array<Vertex, 3> projected_vertices = {};
    std::array<vec3, 3> ndc_points = {};
    std::array<vec2, 3> screen_points = {};
    std::array<float, 3> depth_z = {};

    AABB2D aabb = {};

    const MaterialRaster &material;

    bool smooth = false;

    FullTriangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        const MaterialRaster &material);

    void calculate_tri_aabb();
};

namespace triangle {
    [[nodiscard]] bool is_edge_top_or_left(const vec2 &p1, const vec2 &p2);
    [[nodiscard]] float edge_cross(const vec2 &a, const vec2 &b, const vec2 &p);
}