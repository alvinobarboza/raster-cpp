#include "model/mesh.h"

#include <utility>

MeshData::MeshData(
    std::vector<Triangle> &tris,
    std::vector<Vec3> &vertices,
    std::vector<Vec3> &normals,
    std::vector<Vec2> &uvs,
    TextureRaster *albedo,
    TextureRaster *normal,
    TextureRaster *specular):
triangles(std::move(tris)),
vertices(std::move(vertices)),
normals(std::move(normals)),
uvs(std::move(uvs)),
albedo(albedo),
normal(normal),
specular(specular)
{
    vertices_word.resize(vertices.size());
    normals_word.resize(normals.size());
}

ModelRaster::ModelRaster(
    const Transforms &transform,
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