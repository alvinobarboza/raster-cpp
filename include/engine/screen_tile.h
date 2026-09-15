#pragma once
#include <vector>

#include "colliders/aabb.h"

class ScreenTile {
    public:
    AABB2D aabb{};
    std::vector<int> triangles_id{};
};
