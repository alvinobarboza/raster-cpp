#include "transforms/transforms.h"

// Inverse transform to accommodate camera, as it doesn't actually move
void Transforms::update_transforms(const bool inverse_transform) {
    rotation_matrix.to_rotation(rotation);
    scale_matrix.to_scale(scale);
    translation_matrix.to_translation(position);
    world_matrix = rotation_matrix * scale_matrix;
    world_matrix = translation_matrix * world_matrix;

    if (inverse_transform)
    {
        transposed_rotation_matrix = rotation_matrix.transpose();
        Matrix4x4 negative_translation_matrix{};
        negative_translation_matrix.to_translation(-position);
        view_matrix = scale_matrix * transposed_rotation_matrix;
        view_matrix = view_matrix * negative_translation_matrix;
    }
}
