#pragma once

#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/VertexArray.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include "Renderer/ScriptableEntity.h"

// Forward declaration
namespace S67 {
class ScriptableEntity;
struct LuaScriptComponent {
  std::string FilePath;
  bool Initialized = false;
  std::filesystem::file_time_type LastWriteTime;
  std::shared_ptr<void> Environment;
};
}

namespace S67 {

struct Transform {
  glm::vec3 Position = {0.0f, 0.0f, 0.0f};
  glm::vec3 Rotation = {0.0f, 0.0f, 0.0f}; // Euler angles
  glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

  glm::mat4 GetTransform() const {
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), {1, 0, 0}) *
        glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), {0, 1, 0}) *
        glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), {0, 0, 1});

    return glm::translate(glm::mat4(1.0f), Position) * rotation *
           glm::scale(glm::mat4(1.0f), Scale);
  }
};

// Forward declaration
class ScriptableEntity;

struct NativeScriptComponent {
  std::string Name;
  ScriptableEntity *Instance = nullptr;

  ScriptableEntity *(*InstantiateScript)(NativeScriptComponent *);
  void (*DestroyScript)(NativeScriptComponent *);

  template <typename T> void Bind(const std::string &name) {
    Name = name;
    InstantiateScript = [](NativeScriptComponent *) {
      return static_cast<ScriptableEntity *>(new T());
    };
    DestroyScript = [](NativeScriptComponent *nsc) {
      delete nsc->Instance;
      nsc->Instance = nullptr;
    };
  }
};

struct Material {
  Ref<Texture2D> AlbedoMap;
  glm::vec2 Tiling = {1.0f, 1.0f};
};

struct MovementSettings {
  float MaxSpeed = 190.0f;        // sv_maxspeed
  float MaxSprintSpeed = 320.0f;  // Custom sprint speed
  float MaxCrouchSpeed = 63.3f;   // sv_maxspeed (crouched) roughly 1/3
  float Acceleration = 5.6f;      // sv_accelerate
  float AirAcceleration = 100.0f; // sv_airaccelerate
  float Friction = 4.8f;          // sv_friction
  float StopSpeed = 100.0f;       // sv_stopspeed
  float JumpVelocity = 268.0f;    // JUMP_VELOCITY
  float Gravity = 800.0f;         // sv_gravity
  float MaxAirWishSpeed = 30.0f;  // MAX_AIR_WISH_SPEED
};

class Entity {
public:
  Entity() = default;
  Entity(const std::string &name, const Ref<VertexArray> &va,
         const Ref<Shader> &shader, const Ref<Texture2D> &texture)
      : Name(name), Mesh(va), MaterialShader(shader) {
    Material.AlbedoMap = texture;
  }

  Transform Transform;
  Ref<VertexArray> Mesh;
  Ref<Shader> MaterialShader;
  Material Material;

  JPH::BodyID PhysicsBody = JPH::BodyID();
  std::string Name = "Entity";
  std::string MeshPath = "Cube";
  bool Collidable = true;
  bool Anchored = false; // If true, object is static (no gravity)
  float CameraFOV = 45.0f;

  MovementSettings Movement;

  std::vector<NativeScriptComponent> Scripts;
  std::vector<LuaScriptComponent> LuaScripts;
  std::vector<std::string> Tags;

  template <typename T> T *GetScript() {
    for (auto &script : Scripts) {
      if (T *instance = dynamic_cast<T *>(script.Instance))
        return instance;
    }
    return nullptr;
  }

  bool HasTag(const std::string &tag) const {
    for (const auto &t : Tags) {
      if (t == tag)
        return true;
    }
    return false;
  }
};

} // namespace S67
