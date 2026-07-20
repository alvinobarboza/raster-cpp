#pragma once
#include <array>

#include "colliders/sphere.h"
#include "transforms/Vec3.h"

constexpr size_t TOP_PLANE = 0;
constexpr size_t BOTTOM_PLANE = 1;
constexpr size_t LEFT_PLANE = 2;
constexpr size_t RIGHT_PLANE = 3;
constexpr size_t NEAR_PLANE = 4;
constexpr size_t FAR_PLANE = 5;

constexpr size_t PLANES = 6;

class Plane {
public:
    float distance = 0.0f;
    Vec3 normal = {};

    Plane() = default;
    Plane(const Vec3 &normal, const Vec3 &point);

    [[nodiscard]] float signed_distance_to_point(const Vec3 &point) const;
};

class Frustum {
public:
    std::array<Plane, PLANES> planes;

    Frustum() = default;

    [[nodiscard]] bool is_inside_frustum(const Vec3 &point) const;
    [[nodiscard]] bool is_inside_frustum(const BoundingSphere &sphere) const;
};