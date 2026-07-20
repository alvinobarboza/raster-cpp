#include "colliders/aabb.h"

bool AABB2D::collides(const AABB2D& other) const
{
    return
        this->min.x <= other.max.x &&
        this->max.x >= other.min.x &&
        this->min.y <= other.max.y &&
        this->max.y >= other.min.y;
}