#include "Scene.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Mesh.h"
#include "Physics/PlayerController.h"
#include "Renderer/ScriptableEntity.h"
#include "Texture.h"
#include "Scripting/LuaScriptEngine.h"
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
  bool hasPlayerController = false;
  for (auto &script : player->Scripts) {
    if (script.Name == "PlayerController") {
      hasPlayerController = true;
      break;
    }
  }

  if (!hasPlayerController) {
    NativeScriptComponent nsc;
    nsc.Bind<PlayerController>("PlayerController");
    player->Scripts.push_back(nsc);
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
    for (auto &script : entity->Scripts) {
      if (!script.Instance) {
        S67_CORE_INFO("Instantiating script {0} for entity {1}", script.Name,
                      entity->Name);
        script.Instance = ScriptRegistry::Get().Instantiate(script.Name);
        if (script.Instance) {
          script.Instance->m_Entity = entity.get();
          script.Instance->OnCreate();
        }
      }
    }

    for (auto &luaScript : entity->LuaScripts) {
      if (!luaScript.Initialized && !luaScript.FilePath.empty()) {
        S67_CORE_INFO("Instantiating Lua script {0} for entity {1}",
                      luaScript.FilePath, entity->Name);
        LuaScriptEngine::OnCreate(entity.get());
        luaScript.Initialized = true;
      }
    }
  }
}

void Scene::OnUpdate(float ts) {
  InstantiateScripts();

  for (auto &entity : m_Entities) {
    for (auto &script : entity->Scripts) {
      if (script.Instance) {
        script.Instance->OnUpdate(ts);
      }
    }

    for (auto &luaScript : entity->LuaScripts) {
      if (luaScript.Initialized) {
        LuaScriptEngine::OnUpdate(entity.get(), ts);
      }
    }
  }
}

} // namespace S67
