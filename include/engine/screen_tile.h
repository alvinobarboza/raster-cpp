#pragma once
#include <vector>

#include "colliders/aabb.h"
#include "model/triangle.h"

//Compressed Sparse Row (CSR)
struct Tile {
    int counter{0};
    int offset{0};
    int cursor_offset{0};
};

struct Grid {
    int width {};
    int height {};
    int total_tiles {};
    int last_tri_count{0};
    std::vector<Tile> tiles {};
    std::vector<int> triangles_id {};
};
