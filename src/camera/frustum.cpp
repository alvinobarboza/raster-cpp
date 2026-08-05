#include "camera/frustum.h"

#include <algorithm>

Plane::Plane(const Vec3 &normal, const Vec3 &point)
{
     const Vec3 normalized = normal.normalized();
     this->normal = normalized;
     distance = normalized * point;
}

float Plane::signed_distance_to_point(const Vec3 &point) const {
     return normal * point - distance;
}

bool Frustum::is_inside_frustum(const Vec3 &point) const {
     return std::ranges::all_of(
          planes.cbegin(),
          planes.cend(),
          [&point](const Plane &plane) {
               return plane.signed_distance_to_point(point) > 0;
     });
}

bool Frustum::is_inside_frustum(const BoundingSphere &sphere) const {
     return std::ranges::all_of(
          planes.cbegin(),
          planes.cend(),
          [&sphere](const Plane &plane) {
               return plane.signed_distance_to_point(sphere.center_world) > -sphere.radius;
     });
}
