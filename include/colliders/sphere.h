#pragma once
#include <vector>

#include "transforms/vec3.h"

class BoundingSphere {
public:
    float radius {};
    Vec3 center {}, center_view_space {};

    BoundingSphere() = default;

    void calculate_boundaries(const std::vector<Vec3> &verts, const Matrix4x4 &scale);

    friend std::ostream &operator<<(std::ostream &os, const BoundingSphere &v);
};
