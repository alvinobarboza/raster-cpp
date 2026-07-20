#pragma once
#include <vector>

#include "transforms/Vec3.h"

class BoundingSphere {
public:
    float radius = 0.0f;
    Vec3 center = {}, center_world = {};

    BoundingSphere() = default;

    void calculate_boundaries(const std::vector<Vec3> &verts, const Matrix4x4 &scale);

    friend std::ostream &operator<<(std::ostream &os, const BoundingSphere &v);
};
