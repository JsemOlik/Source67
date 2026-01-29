#include "Scene.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Mesh.h"
#include "Physics/PlayerController.h"
#include "Renderer/ScriptableEntity.h"
#include "Texture.h"
#include <algorithm>

namespace S67 {

void Scene::EnsurePlayerExists() {
  auto it =
      std::find_if(m_Entities.begin(), m_Entities.end(),
                   [](const Ref<Entity> &e) { return e->Name == "Player"; });

  Ref<Entity> player;

  if (it != m_Entities.end()) {
    player = *it;
    // Move to front if not already
    if (it != m_Entities.begin()) {
      m_Entities.erase(it);
      m_Entities.insert(m_Entities.begin(), player);
    }
  } else {
    // Create new
    player = CreateRef<Entity>();
    player->Name = "Player";
    player->Transform.Position = {0.0f, 2.0f, 0.0f};
    player->Transform.Scale = {1.0f, 1.5f, 1.0f}; // Enforce scale
    m_Entities.insert(m_Entities.begin(), player);
  }

  // Enforce Visuals
  if (!player->Mesh || player->MeshPath != "Cube") {
    player->Mesh = MeshLoader::CreateCube();
    player->MeshPath = "Cube";
  }

  // Enforce Texture (level_icon.png) only if missing
  if (!player->Material.AlbedoMap) {
    auto texture = Texture2D::Create("assets/textures/level_icon.png");
    if (texture) {
      player->Material.AlbedoMap = texture;
    } else {
      S67_CORE_WARN(
          "Failed to load player texture: assets/textures/level_icon.png");
    }
  }

  // Ensure Shader logic:
  // Entity constructor might not set Shader if created empty.
  // We need a shader.
  if (!player->MaterialShader) {
    // Fallback to a known shader path if possible, or standard.
    // Application uses FlatColor.glsl usually. We can try to load it.
    // But paths are relative to executable or project root.
    // We'll rely on "assets/shaders/FlatColor.glsl".
    // Ideally we shouldn't hardcode, but for "Player" default it's safer.
    // Or better: Re-use shader if we can get it? No easy way.
    auto shader =
        Shader::Create(Application::Get()
                           .ResolveAssetPath("assets/shaders/Texture.glsl")
                           .string());
    if (shader) {
      player->MaterialShader = shader;
    } else {
      S67_CORE_WARN(
          "Failed to load player shader: assets/shaders/Texture.glsl");
    }
  }

  // Ensure Physics Body is Invalid (or set to Character?)
  // User didn't ask for physics yet, just visual.

  // Bind PlayerController Script
  if (!player->NativeScript.Instance) {
    player->NativeScript.Bind<PlayerController>();
  }
}

Ref<Entity> Scene::FindEntityByName(const std::string &name) {
  auto it = std::find_if(m_Entities.begin(), m_Entities.end(),
                         [&](const Ref<Entity> &e) { return e->Name == name; });
  if (it != m_Entities.end())
    return *it;
  return nullptr;
}

void Scene::InstantiateScripts() {
  for (auto &entity : m_Entities) {
    auto &nsc = entity->NativeScript;
    if (!nsc.Instance && nsc.InstantiateScript) {
      S67_CORE_INFO("Instantiating script for entity {0}", entity->Name);
      nsc.Instance = nsc.InstantiateScript();
      nsc.Instance->m_Entity = entity.get();
      nsc.Instance->OnCreate();
      S67_CORE_INFO("Script instantiated successfully");
    }
  }
}

void Scene::OnUpdate(float ts) {
  InstantiateScripts();

  for (auto &entity : m_Entities) {
    if (entity->NativeScript.Instance) {
      // S67_CORE_TRACE("Updating script for {0}", entity->Name);
      entity->NativeScript.Instance->OnUpdate(ts);
    }
  }
}

} // namespace S67
