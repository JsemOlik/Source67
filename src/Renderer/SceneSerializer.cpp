#include "SceneSerializer.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Renderer/Mesh.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace S67 {

static std::string Trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (std::string::npos == first)
    return "";
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, (last - first + 1));
}

SceneSerializer::SceneSerializer(Scene *scene, const std::string &projectRoot)
    : m_Scene(scene), m_ProjectRoot(projectRoot) {}

std::string SceneSerializer::MakeRelative(const std::string &path) {
  if (path == "Cube" || path.empty() || path == "None")
    return path;

  std::filesystem::path p(path);
  if (p.is_relative())
    return p.generic_string();

  // 1. Try to make relative to project root
  if (!m_ProjectRoot.empty()) {
    std::filesystem::path root(m_ProjectRoot);
    if (p.string().find(root.string()) != std::string::npos) {
      try {
        return std::filesystem::relative(p, root).generic_string();
      } catch (...) {
      }
    }
  }

  // 2. Try to make relative to engine root
  std::filesystem::path engineRoot = Application::Get().GetEngineAssetsRoot();
  if (!engineRoot.empty()) {
    if (p.string().find(engineRoot.string()) != std::string::npos) {
      try {
        return std::filesystem::relative(p, engineRoot).generic_string();
      } catch (...) {
      }
    }
  }

  return p.generic_string();
}

void SceneSerializer::Serialize(const std::string &filepath) {
  std::filesystem::path path(filepath);
  if (!std::filesystem::exists(path.parent_path())) {
    std::filesystem::create_directories(path.parent_path());
  }

  std::stringstream ss;
  ss << "Scene: Untitled\n";
  ss << "UIPath: " << MakeRelative(m_Scene->GetUIPath()) << "\n";
  ss << "Entities:\n";

  for (auto &entity : m_Scene->GetEntities()) {
    ss << "  - Entity: " << entity->Name << "\n";
    ss << "    Transform:\n";
    ss << "      Position: [" << entity->Transform.Position.x << ", "
       << entity->Transform.Position.y << ", " << entity->Transform.Position.z
       << "]\n";
    ss << "      Rotation: [" << entity->Transform.Rotation.x << ", "
       << entity->Transform.Rotation.y << ", " << entity->Transform.Rotation.z
       << "]\n";
    ss << "      Scale: [" << entity->Transform.Scale.x << ", "
       << entity->Transform.Scale.y << ", " << entity->Transform.Scale.z
       << "]\n";
    ss << "    MeshPath: " << MakeRelative(entity->MeshPath) << "\n";

    if (entity->MaterialShader)
      ss << "    ShaderPath: "
         << MakeRelative(entity->MaterialShader->GetPath()) << "\n";
    else
      ss << "    ShaderPath: None\n";

    if (entity->Material.AlbedoMap)
      ss << "    TexturePath: "
         << MakeRelative(entity->Material.AlbedoMap->GetPath()) << "\n";
    else
      ss << "    TexturePath: None\n";

    if (entity->Material.AlbedoMap)
      ss << "    TextureTiling: [" << entity->Material.Tiling.x << ", "
         << entity->Material.Tiling.y << "]\n";

    ss << "    Collidable: " << (entity->Collidable ? "true" : "false") << "\n";
    ss << "    Anchored: " << (entity->Anchored ? "true" : "false") << "\n";

    if (entity->Name == "Player") {
      ss << "    CameraFOV: " << entity->CameraFOV << "\n";
      ss << "    Movement:\n";
      ss << "      MaxSpeed: " << entity->Movement.MaxSpeed << "\n";
      ss << "      MaxSprintSpeed: " << entity->Movement.MaxSprintSpeed << "\n";
      ss << "      MaxCrouchSpeed: " << entity->Movement.MaxCrouchSpeed << "\n";
      ss << "      Acceleration: " << entity->Movement.Acceleration << "\n";
      ss << "      AirAcceleration: " << entity->Movement.AirAcceleration
         << "\n";
      ss << "      Friction: " << entity->Movement.Friction << "\n";
      ss << "      StopSpeed: " << entity->Movement.StopSpeed << "\n";
      ss << "      JumpVelocity: " << entity->Movement.JumpVelocity << "\n";
      ss << "      Gravity: " << entity->Movement.Gravity << "\n";
      ss << "      MaxAirWishSpeed: " << entity->Movement.MaxAirWishSpeed
         << "\n";
    }
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

bool SceneSerializer::Deserialize(const std::string &filepath) {
  std::ifstream fin(filepath);
  if (!fin.is_open()) {
    S67_CORE_ERROR("Failed to open file '{0}' for loading", filepath);
    return false;
  }

  m_Scene->Clear();

  std::string line;
  Ref<Entity> currentEntity = nullptr;

  while (std::getline(fin, line)) {
    if (line.find("UIPath:") != std::string::npos) {
      std::string path = Trim(line.substr(line.find(":") + 1));
      m_Scene->SetUIPath(path);
    } else if (line.find("- Entity:") != std::string::npos) {
      std::string name = Trim(line.substr(line.find(":") + 2));
      currentEntity = CreateRef<Entity>();
      currentEntity->Name = name;
      m_Scene->AddEntity(currentEntity);
    } else if (currentEntity) {
      if (line.find("Position:") != std::string::npos) {
        float x, y, z;
        sscanf(line.c_str(), "      Position: [%f, %f, %f]", &x, &y, &z);
        currentEntity->Transform.Position = {x, y, z};
      } else if (line.find("Rotation:") != std::string::npos) {
        float x, y, z;
        sscanf(line.c_str(), "      Rotation: [%f, %f, %f]", &x, &y, &z);
        currentEntity->Transform.Rotation = {x, y, z};
      } else if (line.find("Scale:") != std::string::npos) {
        float x, y, z;
        sscanf(line.c_str(), "      Scale: [%f, %f, %f]", &x, &y, &z);
        currentEntity->Transform.Scale = {x, y, z};
      } else if (line.find("MeshPath:") != std::string::npos) {
        std::string path = Trim(line.substr(line.find(":") + 2));
        if (path == "Cube") {
          currentEntity->MeshPath = "Cube";
          currentEntity->Mesh = Application::Get().GetCubeMesh();
        } else if (path != "None") {
          currentEntity->MeshPath =
              std::filesystem::path(path).make_preferred().string();
          std::string resolvedPath =
              Application::Get()
                  .ResolveAssetPath(currentEntity->MeshPath)
                  .string();

          if (std::filesystem::path(resolvedPath).extension() == ".obj")
            currentEntity->Mesh = MeshLoader::LoadOBJ(resolvedPath);
          else if (std::filesystem::path(resolvedPath).extension() == ".stl")
            currentEntity->Mesh = MeshLoader::LoadSTL(resolvedPath);
        } else {
          currentEntity->MeshPath = path;
        }
      } else if (line.find("ShaderPath:") != std::string::npos) {
        std::string path = Trim(line.substr(line.find(":") + 2));
        if (path != "None") {
          std::string resolvedPath =
              Application::Get().ResolveAssetPath(path).string();

          // Reuse default shader if path matches
          auto defaultShader = Application::Get().GetDefaultShader();
          if (defaultShader &&
              (path.find("Lighting.glsl") != std::string::npos)) {
            currentEntity->MaterialShader = defaultShader;
          } else {
            auto shader = Shader::Create(resolvedPath);
            if (shader) {
              currentEntity->MaterialShader = shader;
            } else {
              S67_CORE_WARN("Failed to load shader: {0}", resolvedPath);
            }
          }
        }
      } else if (line.find("TexturePath:") != std::string::npos) {
        std::string path = Trim(line.substr(line.find(":") + 2));
        if (path != "None") {
          std::string resolvedPath =
              Application::Get().ResolveAssetPath(path).string();

          // Reuse default texture if path matches
          auto defaultTex = Application::Get().GetDefaultTexture();
          if (defaultTex &&
              (path.find("Checkerboard.png") != std::string::npos)) {
            currentEntity->Material.AlbedoMap = defaultTex;
          } else {
            auto texture = Texture2D::Create(resolvedPath);
            if (texture) {
              currentEntity->Material.AlbedoMap = texture;
            } else {
              S67_CORE_WARN("Failed to load texture: {0}", resolvedPath);
            }
          }
        }
      } else if (line.find("TextureTiling:") != std::string::npos) {
        float x, y;
        if (sscanf(line.c_str(), "      TextureTiling: [%f, %f]", &x, &y) == 2)
          currentEntity->Material.Tiling = {x, y};
      } else if (line.find("Collidable:") != std::string::npos) {
        std::string val = Trim(line.substr(line.find(":") + 2));
        currentEntity->Collidable = (val.find("true") != std::string::npos);
      } else if (line.find("Anchored:") != std::string::npos) {
        std::string val = Trim(line.substr(line.find(":") + 2));
        currentEntity->Anchored = (val.find("true") != std::string::npos);
      } else if (line.find("CameraFOV:") != std::string::npos) {
        float fov;
        if (sscanf(line.c_str(), "      CameraFOV: %f", &fov) == 1)
          currentEntity->CameraFOV = fov;
      } else if (line.find("MaxSpeed:") != std::string::npos) {
        sscanf(line.c_str(), "      MaxSpeed: %f",
               &currentEntity->Movement.MaxSpeed);
      } else if (line.find("MaxSprintSpeed:") != std::string::npos) {
        sscanf(line.c_str(), "      MaxSprintSpeed: %f",
               &currentEntity->Movement.MaxSprintSpeed);
      } else if (line.find("MaxCrouchSpeed:") != std::string::npos) {
        sscanf(line.c_str(), "      MaxCrouchSpeed: %f",
               &currentEntity->Movement.MaxCrouchSpeed);
      } else if (line.find("Acceleration:") != std::string::npos) {
        // Distinguish from AirAcceleration
        if (line.find("AirAcceleration:") == std::string::npos) {
          sscanf(line.c_str(), "      Acceleration: %f",
                 &currentEntity->Movement.Acceleration);
        } else {
          sscanf(line.c_str(), "      AirAcceleration: %f",
                 &currentEntity->Movement.AirAcceleration);
        }
      } else if (line.find("Friction:") != std::string::npos) {
        sscanf(line.c_str(), "      Friction: %f",
               &currentEntity->Movement.Friction);
      } else if (line.find("StopSpeed:") != std::string::npos) {
        sscanf(line.c_str(), "      StopSpeed: %f",
               &currentEntity->Movement.StopSpeed);
      } else if (line.find("JumpVelocity:") != std::string::npos) {
        sscanf(line.c_str(), "      JumpVelocity: %f",
               &currentEntity->Movement.JumpVelocity);
      } else if (line.find("Gravity:") != std::string::npos) {
        sscanf(line.c_str(), "      Gravity: %f",
               &currentEntity->Movement.Gravity);
      } else if (line.find("MaxAirWishSpeed:") != std::string::npos) {
        sscanf(line.c_str(), "      MaxAirWishSpeed: %f",
               &currentEntity->Movement.MaxAirWishSpeed);
      }
    }
  }

  S67_CORE_INFO("Scene loaded from '{0}'", filepath);
  return true;
}

} // namespace S67
