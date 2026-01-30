#include "SceneSerializer.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Renderer/Mesh.h"
#include "Renderer/ScriptRegistry.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::ordered_json;

namespace S67 {

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

  json root;
  root["Scene"] = "Untitled";

  json entities = json::array();
  for (auto &entity : m_Scene->GetEntities()) {
    json e;
    e["Entity"] = entity->Name;

    json transform;
    transform["Position"] = {entity->Transform.Position.x,
                             entity->Transform.Position.y,
                             entity->Transform.Position.z};
    transform["Rotation"] = {entity->Transform.Rotation.x,
                             entity->Transform.Rotation.y,
                             entity->Transform.Rotation.z};
    transform["Scale"] = {entity->Transform.Scale.x, entity->Transform.Scale.y,
                          entity->Transform.Scale.z};
    e["Transform"] = transform;

    e["MeshPath"] = MakeRelative(entity->MeshPath);
    e["ShaderPath"] = entity->MaterialShader
                          ? MakeRelative(entity->MaterialShader->GetPath())
                          : "None";
    e["TexturePath"] = entity->Material.AlbedoMap
                           ? MakeRelative(entity->Material.AlbedoMap->GetPath())
                           : "None";

    if (entity->Material.AlbedoMap) {
      e["TextureTiling"] = {entity->Material.Tiling.x,
                            entity->Material.Tiling.y};
    }

    e["Collidable"] = entity->Collidable;
    e["Anchored"] = entity->Anchored;

    // Tags
    if (!entity->Tags.empty()) {
      e["Tags"] = entity->Tags;
    }

    // Native Scripts
    if (!entity->Scripts.empty()) {
      json scripts = json::array();
      for (const auto &script : entity->Scripts) {
        json s;
        s["Name"] = script.Name;
        scripts.push_back(s);
      }
      e["Scripts"] = scripts;
    }

    // Lua Scripts
    if (!entity->LuaScripts.empty()) {
      json luaScripts = json::array();
      for (auto &script : entity->LuaScripts) {
        luaScripts.push_back(MakeRelative(script.FilePath));
      }
      e["LuaScripts"] = luaScripts;
    }

    if (entity->Name == "Player") {
      e["CameraFOV"] = entity->CameraFOV;
      json movement;
      movement["MaxSpeed"] = entity->Movement.MaxSpeed;
      movement["MaxSprintSpeed"] = entity->Movement.MaxSprintSpeed;
      movement["MaxCrouchSpeed"] = entity->Movement.MaxCrouchSpeed;
      movement["Acceleration"] = entity->Movement.Acceleration;
      movement["AirAcceleration"] = entity->Movement.AirAcceleration;
      movement["Friction"] = entity->Movement.Friction;
      movement["StopSpeed"] = entity->Movement.StopSpeed;
      movement["JumpVelocity"] = entity->Movement.JumpVelocity;
      movement["Gravity"] = entity->Movement.Gravity;
      movement["MaxAirWishSpeed"] = entity->Movement.MaxAirWishSpeed;
      e["Movement"] = movement;
    }

    entities.push_back(e);
  }
  root["Entities"] = entities;

  std::ofstream fout(filepath);
  if (fout.is_open()) {
    fout << root.dump(2); // Indent with 2 spaces
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

  std::stringstream ss;
  ss << fin.rdbuf();
  std::string content = ss.str();

  // Try JSON first
  try {
    json data = json::parse(content);
    m_Scene->Clear();

    if (data.contains("Entities")) {
      for (auto &e : data["Entities"]) {
        Ref<Entity> entity = CreateRef<Entity>();
        entity->Name = e.value("Entity", "Unnamed Entity");

        if (e.contains("Transform")) {
          auto &t = e["Transform"];
          if (t.contains("Position")) {
            entity->Transform.Position = {t["Position"][0], t["Position"][1],
                                          t["Position"][2]};
          }
          if (t.contains("Rotation")) {
            entity->Transform.Rotation = {t["Rotation"][0], t["Rotation"][1],
                                          t["Rotation"][2]};
          }
          if (t.contains("Scale")) {
            entity->Transform.Scale = {t["Scale"][0], t["Scale"][1],
                                       t["Scale"][2]};
          }
        }

        entity->MeshPath = e.value("MeshPath", "");
        if (entity->MeshPath == "Cube") {
          entity->Mesh = Application::Get().GetCubeMesh();
        } else if (entity->MeshPath != "" && entity->MeshPath != "None") {
          entity->MeshPath =
              std::filesystem::path(entity->MeshPath).make_preferred().string();
          std::string resolvedPath =
              Application::Get().ResolveAssetPath(entity->MeshPath).string();

          if (std::filesystem::path(resolvedPath).extension() == ".obj")
            entity->Mesh = MeshLoader::LoadOBJ(resolvedPath);
          else if (std::filesystem::path(resolvedPath).extension() == ".stl")
            entity->Mesh = MeshLoader::LoadSTL(resolvedPath);
        }

        std::string shaderPath = e.value("ShaderPath", "None");
        if (shaderPath != "None") {
          std::string resolvedPath =
              Application::Get().ResolveAssetPath(shaderPath).string();
          auto defaultShader = Application::Get().GetDefaultShader();
          if (defaultShader &&
              (shaderPath.find("Lighting.glsl") != std::string::npos)) {
            entity->MaterialShader = defaultShader;
          } else {
            auto shader = Shader::Create(resolvedPath);
            if (shader)
              entity->MaterialShader = shader;
          }
        }

        std::string texPath = e.value("TexturePath", "None");
        if (texPath != "None") {
          std::string resolvedPath =
              Application::Get().ResolveAssetPath(texPath).string();
          auto defaultTex = Application::Get().GetDefaultTexture();
          if (defaultTex &&
              (texPath.find("Checkerboard.png") != std::string::npos)) {
            entity->Material.AlbedoMap = defaultTex;
          } else {
            auto texture = Texture2D::Create(resolvedPath);
            if (texture)
              entity->Material.AlbedoMap = texture;
          }
        }

        if (e.contains("TextureTiling")) {
          entity->Material.Tiling = {e["TextureTiling"][0],
                                     e["TextureTiling"][1]};
        }

        entity->Collidable = e.value("Collidable", false);
        entity->Anchored = e.value("Anchored", false);

        // Tags
        if (e.contains("Tags")) {
          for (auto &tag : e["Tags"]) {
            entity->Tags.push_back(tag.get<std::string>());
          }
        }

            entity->Scripts.push_back(nsc);
          }
        }

        // Lua Scripts
        if (e.contains("LuaScripts")) {
          for (auto &luaPath : e["LuaScripts"]) {
            std::string path = luaPath.get<std::string>();
            if (!path.empty()) {
              std::string resolvedPath =
                  Application::Get().ResolveAssetPath(path).string();
              entity->LuaScripts.push_back({resolvedPath, false});
            }
          }
        }

        if (entity->Name == "Player" && e.contains("Movement")) {
          entity->CameraFOV = e.value("CameraFOV", 45.0f);
          auto &m = e["Movement"];
          entity->Movement.MaxSpeed = m.value("MaxSpeed", 10.0f);
          entity->Movement.MaxSprintSpeed = m.value("MaxSprintSpeed", 20.0f);
          entity->Movement.MaxCrouchSpeed = m.value("MaxCrouchSpeed", 5.0f);
          entity->Movement.Acceleration = m.value("Acceleration", 50.0f);
          entity->Movement.AirAcceleration = m.value("AirAcceleration", 20.0f);
          entity->Movement.Friction = m.value("Friction", 6.0f);
          entity->Movement.StopSpeed = m.value("StopSpeed", 1.0f);
          entity->Movement.JumpVelocity = m.value("JumpVelocity", 5.0f);
          entity->Movement.Gravity = m.value("Gravity", 9.81f);
          entity->Movement.MaxAirWishSpeed = m.value("MaxAirWishSpeed", 30.0f);
        }

        m_Scene->AddEntity(entity);
      }
    }

    S67_CORE_INFO("Scene loaded from '{0}'", filepath);
    return true;
  } catch (const std::exception &e) {
    S67_CORE_ERROR("JSON parsing failed for level '{0}': {1}", filepath,
                   e.what());
    return false;
  }
}

} // namespace S67
