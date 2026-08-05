#pragma once
#include <vector>

#include "transforms/vec3.h"

class BoundingSphere {
public:
    float radius = 0.0f;
    vec3 center = {}, center_world = {};

    BoundingSphere() = default;

    void calculate_boundaries(const std::vector<vec3> &verts, const matrix4x4 &scale);

    friend std::ostream &operator<<(std::ostream &os, const BoundingSphere &v);
};
