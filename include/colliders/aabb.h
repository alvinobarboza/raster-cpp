#pragma once
#include "transforms/vec2.h"

class AABB2D {
public:
    vec2 min = {}, max = {};

    AABB2D() = default;
    AABB2D(const vec2& min, const vec2& max) : min(min), max(max) {};

    [[nodiscard]] bool collides(const AABB2D& other) const;
};
