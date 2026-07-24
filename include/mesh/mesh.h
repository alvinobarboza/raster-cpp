#pragma once
#include <memory>
#include <vector>

#include "triangle.h"
#include "colliders/sphere.h"
#include "transforms/Transforms.h"

class MeshData {
public:
    std::vector<Triangle> triangles = {};
    std::vector<Vec3> vertices = {}, vertices_word = {};
    std::vector<Vec3> normals = {}, normals_word = {};
    std::vector<Vec2> uvs = {};
    TextureRaster *albedo = nullptr;
    TextureRaster *normal = nullptr;
    TextureRaster *specular = nullptr;

    MeshData() = default;
    MeshData(
        std::vector<Triangle> &tris,
        std::vector<Vec3> &vertices,
        std::vector<Vec3> &normals,
        std::vector<Vec2> &uvs,
        TextureRaster *albedo,
        TextureRaster *normal,
        TextureRaster *specular);
};

class ModelRaster {
public:
    Transforms transforms = {};
    BoundingSphere boundingSphere = {};
    std::unique_ptr<MeshData> meshData = nullptr;

    ModelRaster() = default;
    ModelRaster(const Transforms &transform, std::unique_ptr<MeshData> meshData);

    void update_transforms();
};