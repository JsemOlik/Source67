#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Events/Event.h"
#include "Renderer/Camera.h"
#include "Renderer/Entity.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <glm/glm.hpp>

namespace S67 {

class PlayerController {
public:
  PlayerController(Ref<PerspectiveCamera> camera);
  ~PlayerController();

  void OnUpdate(Timestep ts);
  void OnEvent(Event &e);
  void Reset(const glm::vec3 &startPos);
  void ReinitializeCharacter();

  void SetPosition(const glm::vec3 &position);
  void SetRotation(float yaw, float pitch);
  glm::vec3 GetPosition() const;
  glm::vec3 GetVelocity() const;
  float GetSpeed() const;
  float GetYaw() const { return m_Yaw; }
  float GetPitch() const { return m_Pitch; }

  void SetSettings(const MovementSettings &settings) { m_Settings = settings; }

  // Movement State updates
  void UpdateCrouch(float dt);
  void UpdateSprint(float dt);

private:
  void HandleInput(float dt);
  glm::vec3 GetForwardVector(float yaw, float pitch);
  glm::vec3 GetRightVector(float yaw);

private:
  Ref<PerspectiveCamera> m_Camera;
  JPH::Ref<JPH::CharacterVirtual> m_Character;
  glm::vec3 m_Position = {0.0f, 2.0f, 0.0f};

  MovementSettings m_Settings;

  // Source Movement Constants (HU)
  static constexpr float HU_TO_METERS = 1.0f / 39.97f;

  static constexpr float SPRINT_DURATION = 8.0f;
  static constexpr float SPRINT_RECOVERY = 8.0f;

  // Movement State
  bool m_IsSprinting = false;
  float m_SprintRemaining = SPRINT_DURATION;
  float m_SprintRecoveryTime = 0.0f;

  bool m_IsCrouching = false;
  float m_CrouchTransition =
      1.0f; // 1.0 = standing, 0.0 = crouched (inverted for ease of mix)

  // Input State
  float m_ForwardInput = 0.0f;
  float m_SideInput = 0.0f;
  bool m_JumpPressed = false;
  bool m_SprintPressed = false;
  bool m_CrouchPressed = false;

  float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
  bool m_FirstMouse = true;
  float m_Pitch = 0.0f, m_Yaw = -90.0f;
};

} // namespace S67
