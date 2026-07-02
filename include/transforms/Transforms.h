#pragma once
#include "transforms/Vec3.h"
#include "transforms/Matrix4x4.h"

class Transforms {
public:
    Vec3 scale;
    Vec3 rotation;
    Vec3 position;
    Vec3 forward_direction;

    Matrix4x4 scale_matrix;
    Matrix4x4 rotation_matrix;
    Matrix4x4 translation_matrix;
    Matrix4x4 transformation_matrix;

    Transforms(
        const Vec3 scale,
        const Vec3 rotation,
        const Vec3 position,
        const Vec3 forward_direction):
    scale(scale),
    rotation(rotation),
    position(position),
    forward_direction(forward_direction){};

    void update_transforms(bool inverse_transform = false);
};
