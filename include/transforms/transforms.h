#pragma once
#include "transforms/vec3.h"
#include "transforms/matrix4x4.h"

class transforms {
public:
    vec3 scale = {};
    vec3 rotation = {};
    vec3 position = {};
    vec3 forward_direction = {};

    matrix4x4 scale_matrix = {};
    matrix4x4 rotation_matrix = {};
    matrix4x4 translation_matrix = {};
    matrix4x4 transformation_matrix = {};

    transforms() = default;
    transforms(
        const vec3 scale,
        const vec3 rotation,
        const vec3 position,
        const vec3 forward_direction):
    scale(scale),
    rotation(rotation),
    position(position),
    forward_direction(forward_direction){};

    void update_transforms(bool inverse_transform = false);
};
