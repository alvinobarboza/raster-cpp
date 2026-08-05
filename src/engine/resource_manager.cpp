#include <algorithm>
#include <fstream>
#include <sstream>

#include "engine/resourse_manager.h"

ModelRaster* ResourceManager::LoadModel(const std::string& path)
{
    std::ifstream ifs(path);
    if(!ifs.is_open())
    {
        std::cerr << "Error opening model file: " << path << std::endl;
        return nullptr;
    }

    std::vector<Triangle> tris = {};
    std::vector<Vec3> verts = {};
    std::vector<Vec3> normals = {};
    std::vector<Vec2> uvs = {};

    std::string line;
    std::string header;
    while(getline(ifs, line))
    {
        std::stringstream ss(line);
        Vec3 v{};
        Vec2 uv{};
        Vec3 normal{};

        ss >> header;
        if (header == "v")
        {
            ss >> v.x >> v.y >> v.z;
            verts.push_back(v);
            std::cout << "Vert: " << v << std::endl;
        }
        else if (header == "vn")
        {
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            std::cout << "Normal: "  << normal << std::endl;
        }
        else if (header == "vt")
        {
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
            std::cout << "UV: "  << uv << std::endl;
        }
        else if (header == "f")
        {
            Triangle tri{};
            for (int i = 0; i < 3; i++)
            {
                std::string face_data;
                ss >> face_data;
                std::ranges::replace(face_data, '/', ' ');
                std::stringstream f_data(face_data);

                switch (i) {
                    case 0:
                        f_data >> tri.v1 >> tri.u1 >> tri.n1;
                        break;
                    case 1:
                        f_data >> tri.v2 >> tri.u2 >> tri.n2;
                        break;
                    case 2:
                        f_data >> tri.v3 >> tri.u3 >> tri.n3;
                        break;
                    default: ;
                }
            }
            tris.push_back(tri);
            std::cout << "Faces: " << std::endl;
            std::cout << tri.v1 << " " << tri.u1 << " " << tri.n1 << std::endl;
            std::cout << tri.v2 << " " << tri.u2 << " " << tri.n2 << std::endl;
            std::cout << tri.v3 << " " << tri.u3 << " " << tri.n3 << std::endl;
        }
    }
    ifs.close();

    auto mesh = std::make_unique<MeshData>(tris, verts, normals, uvs);
    const auto scale = Vec3(1.0f, 1.0f, 1.0f);
    const auto position = Vec3(1.0f, 1.0f, 1.0f);
    const auto rotation = Vec3(0.0f, 0.0f, 0.0f);
    const auto forward = Vec3(0.0f, 0.0f, 1.0f);
    const auto transform = Transforms(scale, position, rotation, forward);
    auto model = std::make_unique<ModelRaster>(transform, std::move(mesh));
    auto ptr = model.get();
    models.push_back(std::move(model));
    return ptr;
}

MaterialRaster* ResourceManager::LoadMaterial(const std::string& path)
{
    return nullptr;
}

std::vector<ModelRaster*> ResourceManager::LoadScene(const std::string& path)
{
    return {};
}