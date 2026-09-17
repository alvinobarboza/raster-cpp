#pragma once
#include <vector>

#include "colliders/aabb.h"
#include "model/triangle.h"

class ScreenTile {
    public:
    AABB2D aabb{};
    std::vector<int> triangles_id{};
    bool is_active{false};

    void bin_triangles(const std::vector<FullTriangle> &triangles) noexcept;
};
