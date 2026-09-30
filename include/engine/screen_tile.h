#pragma once
#include <vector>

#include "colliders/aabb.h"
#include "model/triangle.h"

struct Tile {
    int offset_x {0}, offset_y{0};
    std::vector<int> triangles_id {};
};

struct Grid {
    int width {};
    int height {};
    int total_tiles {};
    std::vector<Tile> tiles {};
};
