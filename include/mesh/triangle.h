#pragma once

#include <array>
#include <vector>

#include "colliders/aabb.h"
#include "material/texture.h"
#include "transforms/Vec2.h"
#include "transforms/Vec3.h"

class Triangle {
public:
    int v1 = 0, v2 = 0, v3 = 0;
    int u1 = 0, u2 = 0, u3 = 0;
    int n1 = 0, n2 = 0, n3 = 0;
    bool smooth = false;

    [[nodiscard]] bool is_back_facing(const std::vector<Vec3> &vertices, const std::vector<Vec3> &normals) const;
};

struct Vertex {
    Vec3 point;
    Vec3 normal;
    Vec2 uv;
};

class FullTriangle {
public:
    std::array<Vertex, 3> vertices = {};
    std::array<Vec3, 3> points_projected = {};
    std::array<Vec3, 3> normals_rotated = {};
    std::array<Vec2, 3> uvs_projected = {};
    std::array<Vec2, 3> screen_points = {};
    std::array<float, 3> depth_z = {};

    AABB2D aabb = {};

    Texture &albedo;
    Texture &normal;
    Texture &specular;

    bool smooth = false;

    FullTriangle(
        const Vertex &v1,
        const Vertex &v2,
        const Vertex &v3,
        Texture &albedo,
        Texture &normal,
        Texture &specular);
};

namespace triangle {
    [[nodiscard]] bool is_edge_top_or_left(const Vec2 &p1, const Vec2 &p2);
    [[nodiscard]] float edge_cross(const Vec2 &a, const Vec2 &b, const Vec2 &p);
}