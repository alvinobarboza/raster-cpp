#include "colliders/aabb.h"

bool AABB2D::collides(const AABB2D& other) const
{
    return
        this->min.x <= other.max.x &&
        this->max.x >= other.min.x &&
        this->min.y <= other.max.y &&
        this->max.y >= other.min.y;
}

std::ostream &operator<<(std::ostream &os, const AABB2D& aabb)
{
    os << "min: " << aabb.min << ", max: " << aabb.max;
    return os;
}