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
    std::vector<vec3> vertices = {}, vertices_word = {};
    std::vector<vec3> normals = {}, normals_word = {};
    std::vector<vec2> uvs = {};
    std::vector<MaterialRaster> materials = {};

    MeshData() = default;
    MeshData(
        std::vector<Triangle> &tris,
        std::vector<vec3> &vertices,
        std::vector<vec3> &normals,
        std::vector<vec2> &uvs);
};

class ModelRaster {
public:
    transforms transforms = {};
    BoundingSphere boundingSphere = {};
    std::unique_ptr<MeshData> meshData = nullptr;

    ModelRaster() = default;
    ModelRaster(const transforms &transform, std::unique_ptr<MeshData> meshData);

    void update_transforms();
};