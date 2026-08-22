#include "model/mesh.h"

#include <utility>

MeshData::MeshData(
    std::vector<Triangle> &tris,
    std::vector<Vec3> &vertices,
    std::vector<Vec3> &normals,
    std::vector<Vec2> &uvs,
    std::vector<MaterialRaster> &materials):
triangles(std::move(tris)),
vertices(std::move(vertices)),
normals(std::move(normals)),
uvs(std::move(uvs)),
materials(std::move(materials))
{
    vertices_word.resize(this->vertices.size());
    normals_word.resize(this->normals.size());
}

ModelRaster::ModelRaster(
    const Transforms &transform,
    MeshData meshData):
transforms(transform),
meshData(std::move(meshData))
{
    update_transforms();
}

void ModelRaster::update_transforms()
{
    transforms.update_transforms();
    boundingSphere.calculate_boundaries(meshData.vertices, transforms.scale_matrix);
    boundingSphere.center_world = boundingSphere.center * transforms.transformation_matrix;
}