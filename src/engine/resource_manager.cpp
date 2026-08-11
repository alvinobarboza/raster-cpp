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
            // std::cout << "Vert: " << v << std::endl;
        }
        else if (header == "vn")
        {
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            // std::cout << "Normal: "  << normal << std::endl;
        }
        else if (header == "vt")
        {
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
            // std::cout << "UV: "  << uv << std::endl;
        }
        else if (header == "f")
        {
            std::string temp;

            int v_temp, u, n;
            std::vector<int> vid{}, uid{}, nid{};
            while(ss >> temp) {
                std::ranges::replace(temp, '/', ' ');
                std::stringstream f_data(temp);
                f_data >> v_temp >> u >> n;
                vid.push_back(v_temp);
                uid.push_back(u);
                nid.push_back(n);
            }

            Triangle tri{};
            for(int i = 1; i < vid.size()-1; i++) {
                tri.v1 = vid[0] - 1, tri.u1 = uid[0] - 1, tri.n1 = nid[0] - 1;
                tri.v2 = vid[i] - 1, tri.u2 = uid[i] - 1, tri.n2 = nid[i] - 1;
                tri.v3 = vid[i+1] - 1, tri.u3 = uid[i+1] - 1, tri.n3 = nid[i+1] - 1;
                tris.push_back(tri);
            }
        }
    }
    ifs.close();

    auto mesh = std::make_unique<MeshData>(tris, verts, normals, uvs);
    const auto scale = Vec3(1.0f, 1.0f, 1.0f);
    const auto position = Vec3(1.0f, 1.0f, 1.0f);
    const auto rotation = Vec3(0.0f, 0.0f, 0.0f);
    const auto forward = Vec3(0.0f, 0.0f, 1.0f);
    const auto transform = Transforms(scale, rotation, position, forward);
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