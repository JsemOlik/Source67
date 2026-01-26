#include "SceneSerializer.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Core/Logger.h"

namespace S67 {

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene) {}

    void SceneSerializer::Serialize(const std::string& filepath) {
        std::filesystem::path path(filepath);
        if (!std::filesystem::exists(path.parent_path())) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::stringstream ss;
        ss << "Scene: Untitled\n";
        ss << "Entities:\n";

        for (auto& entity : m_Scene->GetEntities()) {
            ss << "  - Entity: " << entity->Name << "\n";
            ss << "    Transform:\n";
            ss << "      Position: [" << entity->Transform.Position.x << ", " << entity->Transform.Position.y << ", " << entity->Transform.Position.z << "]\n";
            ss << "      Rotation: [" << entity->Transform.Rotation.x << ", " << entity->Transform.Rotation.y << ", " << entity->Transform.Rotation.z << "]\n";
            ss << "      Scale: [" << entity->Transform.Scale.x << ", " << entity->Transform.Scale.y << ", " << entity->Transform.Scale.z << "]\n";
            ss << "    MeshPath: " << entity->MeshPath << "\n";
            ss << "    ShaderPath: " << entity->MaterialShader->GetPath() << "\n";
            ss << "    TexturePath: " << entity->Material.AlbedoMap->GetPath() << "\n";
            ss << "    Collidable: " << (entity->Collidable ? "true" : "false") << "\n";
        }

        std::ofstream fout(filepath);
        if (fout.is_open()) {
            fout << ss.str();
            fout.close();
            S67_CORE_INFO("Scene saved to '{0}'", filepath);
        } else {
            S67_CORE_ERROR("Failed to open file '{0}' for saving", filepath);
        }
    }

    bool SceneSerializer::Deserialize(const std::string& filepath) {
        std::ifstream fin(filepath);
        if (!fin.is_open()) {
            S67_CORE_ERROR("Failed to open file '{0}' for loading", filepath);
            return false;
        }

        m_Scene->Clear();

        std::string line;
        Ref<Entity> currentEntity = nullptr;

        while (std::getline(fin, line)) {
            if (line.find("- Entity:") != std::string::npos) {
                std::string name = line.substr(line.find(":") + 2);
                currentEntity = CreateRef<Entity>();
                currentEntity->Name = name;
                m_Scene->AddEntity(currentEntity);
            } else if (currentEntity) {
                if (line.find("Position:") != std::string::npos) {
                    float x, y, z;
                    sscanf(line.c_str(), "      Position: [%f, %f, %f]", &x, &y, &z);
                    currentEntity->Transform.Position = { x, y, z };
                } else if (line.find("Rotation:") != std::string::npos) {
                    float x, y, z;
                    sscanf(line.c_str(), "      Rotation: [%f, %f, %f]", &x, &y, &z);
                    currentEntity->Transform.Rotation = { x, y, z };
                } else if (line.find("Scale:") != std::string::npos) {
                    float x, y, z;
                    sscanf(line.c_str(), "      Scale: [%f, %f, %f]", &x, &y, &z);
                    currentEntity->Transform.Scale = { x, y, z };
                } else if (line.find("MeshPath:") != std::string::npos) {
                    currentEntity->MeshPath = line.substr(line.find(":") + 2);
                } else if (line.find("ShaderPath:") != std::string::npos) {
                    std::string path = line.substr(line.find(":") + 2);
                    currentEntity->MaterialShader = Shader::Create(path);
                } else if (line.find("TexturePath:") != std::string::npos) {
                    std::string path = line.substr(line.find(":") + 2);
                    currentEntity->Material.AlbedoMap = Texture2D::Create(path);
                } else if (line.find("Collidable:") != std::string::npos) {
                    std::string val = line.substr(line.find(":") + 2);
                    currentEntity->Collidable = (val.find("true") != std::string::npos);
                }
            }
        }

        S67_CORE_INFO("Scene loaded from '{0}'", filepath);
        return true;
    }

}
