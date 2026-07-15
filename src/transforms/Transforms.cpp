#include "transforms/Transforms.h"

void Transforms::update_transforms(const bool inverse_transform) {
    rotation_matrix.to_rotation(rotation);

    if (inverse_transform)
        rotation_matrix = rotation_matrix.transpose();
    
    scale_matrix.to_scale(scale);
    translation_matrix.to_translation(!inverse_transform ? position : -position);

    transformation_matrix = !inverse_transform ?
        rotation_matrix * scale_matrix : scale_matrix * rotation_matrix;
    transformation_matrix = !inverse_transform ?
        translation_matrix * transformation_matrix : transformation_matrix * translation_matrix;
}
