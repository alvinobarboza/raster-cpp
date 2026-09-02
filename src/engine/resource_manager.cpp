#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "raylib.h"
#include "engine/resourse_manager.h"

ModelRaster* ResourceManager::load_model(const std::string& path, bool flip_handiness)
{
    std::filesystem::path p(path);
    std::cout << p << std::endl;

    std::ifstream ifs(p);
    if(!ifs.is_open())
    {
        std::cerr << "Error opening model file: " << path << std::endl;
        return nullptr;
    }

    std::vector<Triangle> tris = {};
    std::vector<Vec3> verts = {};
    std::vector<Vec3> normals = {};
    std::vector<Vec2> uvs = {};
    std::vector<MaterialRaster> materials = {};
    bool smooth_shader = false;
    std::string cur_mat_name;
    std::string object_name;

    std::string line;
    std::string header;
    while(getline(ifs, line))
    {
        std::stringstream ss(line);
        Vec3 v{};
        Vec2 uv{};
        Vec3 normal{};

        ss >> header;
        if (header == "o")
        {
            ss >> object_name;
        }
        else if (header == "v")
        {
            ss >> v.x >> v.y >> v.z;
            if (flip_handiness) v.z = -v.z;
            verts.push_back(v);
            // std::cout << "Vert: " << v << std::endl;
        }
        else if (header == "vn")
        {
            ss >> normal.x >> normal.y >> normal.z;
            if (flip_handiness) normal.z = -normal.z;
            normals.push_back(normal);
            // std::cout << "Normal: "  << normal << std::endl;
        }
        else if (header == "vt")
        {
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
            // std::cout << "UV: "  << uv << std::endl;
        }
        else if (header == "s")
        {
            ss >> header;
            if (header == "1") smooth_shader = true;
        }
        else if (header == "usemtl")
        {
            ss >> cur_mat_name;
        }
        else if (header == "f")
        {
            std::string temp;
            int mat_index = -1;

            for (int i = 0; i < materials.size(); i++)
            {
                if (materials[i].name == cur_mat_name)
                {
                    mat_index = i;
                }
            }

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
            tri.smooth = smooth_shader;
            for(int i = 1; i < vid.size()-1; i++) {
                if (flip_handiness)
                {
                    tri.v1 = vid[0] - 1, tri.u1 = uid[0] - 1, tri.n1 = nid[0] - 1;
                    tri.v2 = vid[i+1] - 1, tri.u2 = uid[i+1] - 1, tri.n2 = nid[i+1] - 1;
                    tri.v3 = vid[i] - 1, tri.u3 = uid[i] - 1, tri.n3 = nid[i] - 1;
                }
                else
                {
                    tri.v1 = vid[0] - 1, tri.u1 = uid[0] - 1, tri.n1 = nid[0] - 1;
                    tri.v2 = vid[i] - 1, tri.u2 = uid[i] - 1, tri.n2 = nid[i] - 1;
                    tri.v3 = vid[i+1] - 1, tri.u3 = uid[i+1] - 1, tri.n3 = nid[i+1] - 1;
                }

                tri.material_id = mat_index;
                tris.push_back(tri);
            }
        }
        else if (header == "mtllib")
        {
            std::string mtl_path;
            ss >> mtl_path;
            p.replace_filename(mtl_path);
            materials = load_material(p.string());
        }
    }
    ifs.close();

    const auto mesh = MeshData(tris, verts, normals, uvs, materials);
    const auto scale = Vec3(1.0f, 1.0f, 1.0f);
    const auto position = Vec3(1.0f, 1.0f, 1.0f);
    const auto rotation = Vec3(0.0f, 0.0f, 0.0f);
    const auto forward = Vec3(0.0f, 0.0f, 1.0f);
    const auto transform = Transforms(scale, rotation, position, forward);
    auto model = std::make_unique<ModelRaster>(transform, mesh, object_name);
    auto ptr = model.get();
    models.push_back(std::move(model));
    return ptr;
}

std::vector<MaterialRaster> ResourceManager::load_material(const std::string& path)
{
    std::vector<MaterialRaster> materials = {};
    std::filesystem::path p(path);

    std::ifstream ifs_m(p);
    if(ifs_m.is_open())
    {
        std::cout << p << std::endl;
        std::string m_line;
        std::string m_header;
        std::string mat_name;

        Vec4 diff{-1.0f,-1.0f,-1.0f,1.0f};
        float specular = -1.0f;
        TextureRaster* map_diffuse = nullptr;
        TextureRaster* map_normal = nullptr;
        TextureRaster* map_roughness = nullptr;

        while(getline(ifs_m, m_line))
        {
            if (m_line.empty()) continue;

            std::stringstream m_ss(m_line);
            m_ss >> m_header;
            if(m_header == "newmtl")
            {
                if (!mat_name.empty())
                {
                    materials.emplace_back(
                        mat_name,
                        diff,
                        specular,
                        map_diffuse,
                        map_normal,
                        map_roughness);

                    diff.x = -1.0f, diff.y = -1.0f, diff.z = -1.0f;
                    specular = -1.0f;
                    map_diffuse = nullptr;
                    map_normal = nullptr;
                    map_roughness = nullptr;
                }
                m_ss >> mat_name;
            }
            else if (m_header == "Ns")
            {
                m_ss >> specular;
            }
            else if (m_header == "Kd")
            {
                m_ss >> diff.x >> diff.y >> diff.z;
            }
            else if (m_header == "map_Kd")
            {
                std::string filename;
                m_ss >> filename;
                p.replace_filename(filename);
                map_diffuse = load_texture(p.string());
            }
            else if (m_header == "map_Ns")
            {
                std::string filename;
                m_ss >> filename;
                p.replace_filename(filename);
                map_roughness = load_texture(p.string());
            }
            else if (m_header == "map_Bump")
            {
                std::string filename;
                while (m_ss >> filename) {}
                p.replace_filename(filename);
                map_normal = load_texture(p.string());
            }
        }
        materials.emplace_back(
            mat_name,
            diff,
            specular,
            map_diffuse,
            map_normal,
            map_roughness);

        ifs_m.close();
    }
    return materials;
}

std::vector<ModelRaster*> ResourceManager::load_scene(const std::string& path)
{
    return {};
}

TextureRaster *ResourceManager::load_texture(const std::string &path)
{
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        std::cerr << "Failed to load image: " << path << "\n";
        return nullptr;
    }

    if (img.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }

    auto tex = std::make_unique<TextureRaster>();
    tex->width = img.width;
    tex->height = img.height;
    tex->f_width = static_cast<float>(img.width);
    tex->f_height = static_cast<float>(img.height);

    tex->width_mask = img.width - 1;
    tex->height_mask = img.height - 1;

    const int total_pixels = img.width * img.height;
    tex->buffer.resize(total_pixels);

    const auto* raw_bytes = static_cast<const unsigned char*>(img.data);
    constexpr float inv255 = 1.0f / 255.0f;

    for (int i = 0; i < total_pixels; ++i) {
        const int idx = i * 4;
        tex->buffer[i] = Vec4(
            static_cast<float>(raw_bytes[idx + 0]) * inv255, // R
            static_cast<float>(raw_bytes[idx + 1]) * inv255, // G
            static_cast<float>(raw_bytes[idx + 2]) * inv255, // B
            static_cast<float>(raw_bytes[idx + 3]) * inv255  // A
        );
    }

    UnloadImage(img);

    TextureRaster* ptr = tex.get();
    textures.push_back(std::move(tex));
    return ptr;
}