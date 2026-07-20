#pragma once
#include "transforms/Vec2.h"

class AABB2D {
public:
    Vec2 min = {}, max = {};

    AABB2D() = default;
    AABB2D(const Vec2& min, const Vec2& max) : min(min), max(max) {};

    [[nodiscard]] bool collides(const AABB2D& other) const;
};
