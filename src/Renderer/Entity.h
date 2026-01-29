#pragma once

// Jolt.h must be included first before any other headers
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/VertexArray.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

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

struct Material {
  Ref<Texture2D> AlbedoMap;
  glm::vec2 Tiling = {1.0f, 1.0f};
};

struct MovementSettings {
  float MaxSpeed = 190.0f;       // SPEED_RUN
  float MaxSprintSpeed = 320.0f; // SPEED_SPRINT
  float MaxCrouchSpeed = 63.3f;  // SPEED_CROUCH
  float Acceleration = 5.6f;     // SV_ACCELERATE
  float AirAcceleration =
      12.0f;                   // SV_AIRACCELERATE (Standard Quake/Source value)
  float Friction = 4.8f;       // SV_FRICTION
  float StopSpeed = 100.0f;    // SV_STOPSPEED (sv_stopspeed)
  float JumpVelocity = 268.0f; // JUMP_VELOCITY
  float Gravity = 800.0f;      // GRAVITY
  float MaxAirWishSpeed = 30.0f; // MAX_AIR_WISH_SPEED
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
};

} // namespace S67
