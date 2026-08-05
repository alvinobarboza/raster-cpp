#include "model/mesh.h"

#include <utility>

MeshData::MeshData(
    std::vector<Triangle> &tris,
    std::vector<vec3> &vertices,
    std::vector<vec3> &normals,
    std::vector<vec2> &uvs):
triangles(std::move(tris)),
vertices(std::move(vertices)),
normals(std::move(normals)),
uvs(std::move(uvs))
{
    vertices_word.resize(vertices.size());
    normals_word.resize(normals.size());
}

ModelRaster::ModelRaster(
    const transforms &transform,
    std::unique_ptr<MeshData> meshData):
transforms(transform),
meshData(std::move(meshData))
{
    update_transforms();
}

void ModelRaster::update_transforms()
{
    transforms.update_transforms();
    boundingSphere.calculate_boundaries(meshData->vertices, transforms.scale_matrix);
    boundingSphere.center_world = boundingSphere.center * transforms.transformation_matrix;
}