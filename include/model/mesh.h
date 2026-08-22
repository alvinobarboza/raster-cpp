#pragma once
#include <memory>
#include <vector>

#include "triangle.h"
#include "colliders/sphere.h"
#include "material/material.h"
#include "transforms/transforms.h"

class MeshData {
public:
    std::vector<Triangle> triangles = {};
    std::vector<Vec3> vertices = {}, vertices_word = {};
    std::vector<Vec3> normals = {}, normals_word = {};
    std::vector<Vec2> uvs = {};
    std::vector<MaterialRaster> materials = {};

    MeshData() = default;
    MeshData(
        std::vector<Triangle> &tris,
        std::vector<Vec3> &vertices,
        std::vector<Vec3> &normals,
        std::vector<Vec2> &uvs,
        std::vector<MaterialRaster> &materials);
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