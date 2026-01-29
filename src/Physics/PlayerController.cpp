#include "PlayerController.h"
#include "Core/Input.h"
#include "Events/MouseEvent.h"
#include "Game/Console/ConVar.h" // Include ConVar

#include "Core/Application.h"
#include "Core/Logger.h"
#include "Physics/PhysicsSystem.h"
#include "Renderer/Entity.h"
#include <GLFW/glfw3.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <algorithm>
#include <cmath>
#include <utility>

namespace S67 {

// Console Variables for Player Physics (Valve Naming Convention)
static ConVar sv_maxspeed("sv_maxspeed", "190.0", FCVAR_ARCHIVE | FCVAR_NOTIFY,
                          "Maximum player speed on ground");
static ConVar sv_sprint_speed("sv_sprint_speed", "320.0",
                              FCVAR_ARCHIVE | FCVAR_NOTIFY,
                              "Maximum player speed when sprinting");
static ConVar sv_crouch_speed("sv_crouch_speed", "63.3",
                              FCVAR_ARCHIVE | FCVAR_NOTIFY,
                              "Maximum player speed when crouching");
static ConVar sv_accelerate("sv_accelerate", "5.6",
                            FCVAR_ARCHIVE | FCVAR_NOTIFY,
                            "Ground acceleration setting");
static ConVar sv_airaccelerate("sv_airaccelerate", "100.0",
                               FCVAR_ARCHIVE | FCVAR_NOTIFY,
                               "Air acceleration setting");
static ConVar sv_friction("sv_friction", "4.8", FCVAR_ARCHIVE | FCVAR_NOTIFY,
                          "Ground friction setting");
static ConVar sv_stopspeed("sv_stopspeed", "100.0",
                           FCVAR_ARCHIVE | FCVAR_NOTIFY,
                           "Minimum stopping speed when on ground");
static ConVar sv_jump_velocity("sv_jump_velocity", "268.0",
                               FCVAR_ARCHIVE | FCVAR_NOTIFY,
                               "Initial velocity for jumps");
static ConVar sv_gravity("sv_gravity", "800.0", FCVAR_ARCHIVE | FCVAR_NOTIFY,
                         "Gravity setting");
static ConVar sv_max_air_wishspeed(
    "sv_max_air_wishspeed", "30.0", FCVAR_ARCHIVE | FCVAR_NOTIFY,
    "Maximum speed the player can wish for in air (clamps strafing)");

class PlayerBodyFilter : public JPH::BodyFilter {
public:
  virtual bool ShouldCollide(const JPH::BodyID &inBodyID) const override {
    JPH::BodyInterface &bodyInterface = PhysicsSystem::GetBodyInterface();
    uint64_t userData = bodyInterface.GetUserData(inBodyID);
    if (userData != 0) {
      Entity *entity = (Entity *)userData;
      return entity->Collidable;
    }
    return true;
  }
};

PlayerController::PlayerController() {}

void PlayerController::OnCreate() {
  S67_CORE_INFO("PlayerController::OnCreate Start");
  auto &app = Application::Get();
  S67_CORE_INFO("Got Application instance");
  m_Camera = app.GetCamera(); // Access Global Camera
  if (!m_Camera)
    S67_CORE_ERROR("Camera is null!");
  S67_CORE_INFO("PlayerController::OnCreate Camera Retrieved");
  ReinitializeCharacter();
  S67_CORE_INFO("PlayerController::OnCreate End");
}

PlayerController::~PlayerController() { OnDestroy(); }

void PlayerController::OnDestroy() {
  if (m_Character) {
    // Cleanup Jolt Character if needed, currently raw pointer but Jolt manages
    // it? Actually we new'd it. We should delete it.
    // PhysicsSystem::GetPhysicsSystem().GetBodyInterface().RemoveBody(m_Character->GetBodyID());
    // delete m_Character;
    // For now, minimal cleanup to avoid crashes if Jolt shuts down first.
  }
}

void PlayerController::Reset(const glm::vec3 &startPos) {
  ReinitializeCharacter(); // Ensure character is valid after physics reset
  m_Position = startPos;
  SetPosition(startPos);
  m_Character->SetLinearVelocity(JPH::Vec3::sZero());
  m_Yaw = -90.0f;
  m_Pitch = 0.0f;
  m_Camera->SetYaw(m_Yaw);
  m_Camera->SetPitch(m_Pitch);
  m_FirstMouse = true;

  // Reset Source state
  m_IsSprinting = false;
  m_SprintRemaining = SPRINT_DURATION;
  m_SprintRecoveryTime = 0.0f;
  m_IsCrouching = false;
  m_CrouchTransition = 1.0f;
  m_ForwardInput = 0.0f;
  m_SideInput = 0.0f;
  m_JumpPressed = false;
  m_SprintPressed = false;
  m_CrouchPressed = false;
}

void PlayerController::ReinitializeCharacter() {
  JPH::CharacterVirtualSettings settings;
  settings.mMass = 70.0f;
  settings.mMaxStrength = 100.0f;
  settings.mShape = JPH::RotatedTranslatedShapeSettings(
                        JPH::Vec3(0, 0.9f, 0), JPH::Quat::sIdentity(),
                        new JPH::CapsuleShape(0.9f, 0.3f))
                        .Create()
                        .Get();
  settings.mCharacterPadding = 0.02f;
  settings.mPenetrationRecoverySpeed = 1.0f;
  settings.mPredictiveContactDistance = 0.1f;

  m_Character = new JPH::CharacterVirtual(
      &settings, JPH::RVec3(m_Position.x, m_Position.y, m_Position.z),
      JPH::Quat::sIdentity(), &PhysicsSystem::GetPhysicsSystem());
}

void PlayerController::OnEvent(Event &e) {
  // Using OnUpdate for rotation
}

void PlayerController::OnUpdate(float ts) {
  float dt = ts;

  static float logTimer = 0.0f;
  logTimer += ts;
  if (logTimer >= 1.0f) {
    S67_CORE_INFO("PlayerController Script Updating... (dt={0})", ts);
    logTimer = 0.0f;
  }

  // Sync Settings from Console Variables
  // Safety Check: Only apply if ConVars are initialized (non-zero)
  if (sv_maxspeed.GetFloat() != 0.0f) {
    m_Settings.MaxSpeed = sv_maxspeed.GetFloat();
    m_Settings.MaxSprintSpeed = sv_sprint_speed.GetFloat();
    m_Settings.MaxCrouchSpeed = sv_crouch_speed.GetFloat();
    m_Settings.Acceleration = sv_accelerate.GetFloat();
    m_Settings.AirAcceleration = sv_airaccelerate.GetFloat();
    m_Settings.Friction = sv_friction.GetFloat();
    m_Settings.StopSpeed = sv_stopspeed.GetFloat();
    m_Settings.JumpVelocity = sv_jump_velocity.GetFloat();
    m_Settings.Gravity = sv_gravity.GetFloat();
    m_Settings.MaxAirWishSpeed = sv_max_air_wishspeed.GetFloat();
  }

  // 1. Rotation Update
  std::pair<float, float> mousePos = Input::GetMousePosition();
  float x = mousePos.first;
  float y = mousePos.second;
  if (m_FirstMouse) {
    m_LastMouseX = x;
    m_LastMouseY = y;
    m_FirstMouse = false;
  }

  float xoffset = x - m_LastMouseX;
  float yoffset = m_LastMouseY - y;
  m_LastMouseX = x;
  m_LastMouseY = y;

  float sensitivity = 0.1f;
  m_Yaw += xoffset * sensitivity;
  m_Pitch += yoffset * sensitivity;

  if (m_Pitch > 89.0f)
    m_Pitch = 89.0f;
  if (m_Pitch < -89.0f)
    m_Pitch = -89.0f;

  m_Camera->SetYaw(m_Yaw);
  m_Camera->SetPitch(m_Pitch);

  // 2. Input
  HandleInput(dt);
  UpdateSprint(dt);
  UpdateCrouch(dt);

  // 3. Physics Movement
  // Get Velocity from Jolt (Meters) -> Convert to HU
  JPH::Vec3 jVel = m_Character->GetLinearVelocity();
  glm::vec3 velocity = {jVel.GetX() * METERS_TO_HU, jVel.GetY() * METERS_TO_HU,
                        jVel.GetZ() * METERS_TO_HU};

  CheckGround(dt);

  bool didJump = false;
  if (m_JumpPressed) {
    didJump = CheckJump(velocity, dt);
  }

  if (!didJump && m_Character->GetGroundState() ==
                      JPH::CharacterVirtual::EGroundState::OnGround) {
    GroundMove(velocity, dt);
  } else {
    AirMove(velocity, dt);
  }

  // Convert Velocity HU -> Meters
  JPH::Vec3 newJVel(velocity.x * HU_TO_METERS, velocity.y * HU_TO_METERS,
                    velocity.z * HU_TO_METERS);
  m_Character->SetLinearVelocity(newJVel);

  // 4. Update Character (Collision)
  PlayerBodyFilter bodyFilter;
  m_Character->Update(dt, JPH::Vec3::sZero(), // Gravity applied manually
                      PhysicsSystem::GetBroadPhaseLayerFilter(),
                      PhysicsSystem::GetObjectLayerFilter(), bodyFilter,
                      JPH::ShapeFilter(), m_TempAllocator);

  // 5. Sync Camera Position
  JPH::RVec3 charPos = m_Character->GetPosition();
  float eyeHeight = glm::mix(1.7f, 0.8f, 1.0f - m_CrouchTransition);
  float headBob = 0.0f; // Placeholder for future

  m_Camera->SetPosition(
      {charPos.GetX(), charPos.GetY() + eyeHeight + headBob, charPos.GetZ()});
}

// =========================================================================================
// Source Engine Movement Implementation
// =========================================================================================

void PlayerController::CheckGround(float dt) {
  // Relying on Jolt's Ground State for now, but in a full implementation we
  // might trace explicitly m_Character->Update() handles ground detection.
}

bool PlayerController::CheckJump(glm::vec3 &velocity, float dt) {
  if (m_Character->GetGroundState() ==
      JPH::CharacterVirtual::EGroundState::OnGround) {

    velocity.y = m_Settings.JumpVelocity;

    // Unset flag (no auto-hop unless released and pressed again - handled by
    // Input usually, but here we just consume the press for this frame) Note:
    // If you want bunny hopping, you generally ALLOW holding space or require
    // re-press. Quake/Source usually requires a re-press or scroll wheel spam.
    // For simplicity, we consume it.
    m_JumpPressed = false;

    return true;
  }
  return false;
}

void PlayerController::GroundMove(glm::vec3 &velocity, float dt) {
  Friction(velocity, dt);

  glm::vec3 forward = GetForwardVector(m_Yaw, 0.0f);
  glm::vec3 right = GetRightVector(m_Yaw);

  glm::vec3 wishvel = (forward * m_ForwardInput) + (right * m_SideInput);

  // Normalise direction, scale by speed
  float maxSpeed = m_Settings.MaxSpeed;
  if (m_IsSprinting)
    maxSpeed = m_Settings.MaxSprintSpeed;
  if (m_IsCrouching)
    maxSpeed = m_Settings.MaxCrouchSpeed;

  // In source, wishvel is Input * MaxSpeed directly roughly.
  // We normalized input to -1/1. So we scale by maxSpeed.
  wishvel *= maxSpeed;

  glm::vec3 wishdir = wishvel;
  float wishspeed = glm::length(wishdir);
  if (wishspeed > 0.0f) {
    wishdir /= wishspeed;
  }

  // Cap wishSpeed
  if (wishspeed > maxSpeed)
    wishspeed = maxSpeed;

  // Accelerate
  Accelerate(velocity, wishdir, wishspeed, m_Settings.Acceleration, dt);

  // Add Gravity (Stick to ground)
  // Source doesn't apply full gravity on ground, but Jolt might need a small
  // downward force? Source applies gravity in AirMove usually. GroundMove just
  // handles XY mostly. But if we are on a slope, we might need some Y.
  velocity.y = -10.0f; // Small stick force
}

void PlayerController::AirMove(glm::vec3 &velocity, float dt) {
  glm::vec3 forward = GetForwardVector(m_Yaw, 0.0f);
  glm::vec3 right = GetRightVector(m_Yaw);

  glm::vec3 wishvel = (forward * m_ForwardInput) + (right * m_SideInput);

  // Air control limit
  float maxSpeed = m_Settings.MaxAirWishSpeed;
  // Normally wishspeed is clamped to 30 for air accelerate calculation,
  // but if you strafe, you can go faster.

  // Scale wishvel by MaxSpeed (run speed) to get direction intent
  wishvel *= m_Settings.MaxSpeed;

  glm::vec3 wishdir = wishvel;
  float wishspeed = glm::length(wishdir);
  if (wishspeed > 0.0f) {
    wishdir /= wishspeed;
  }

  // Cap wishspeed for calculation
  if (wishspeed > m_Settings.MaxAirWishSpeed)
    wishspeed = m_Settings.MaxAirWishSpeed;

  AirAccelerate(velocity, wishdir, wishspeed, m_Settings.AirAcceleration, dt);

  // Gravity
  velocity.y -= m_Settings.Gravity * dt;
}

void PlayerController::Friction(glm::vec3 &velocity, float dt) {
  float speed = glm::length(glm::vec3(velocity.x, 0, velocity.z));
  if (speed < 0.1f)
    return;

  float drop = 0.0f;
  float friction = m_Settings.Friction;
  float stopSpeed = m_Settings.StopSpeed;
  float control = (speed < stopSpeed) ? stopSpeed : speed;

  drop += control * friction * dt;

  float newSpeed = speed - drop;
  if (newSpeed < 0)
    newSpeed = 0;
  newSpeed /= speed;

  velocity.x *= newSpeed;
  velocity.z *= newSpeed;
}

void PlayerController::Accelerate(glm::vec3 &velocity, const glm::vec3 &wishdir,
                                  float wishspeed, float accel, float dt) {
  float currentspeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishdir);
  float addspeed = wishspeed - currentspeed;

  if (addspeed <= 0)
    return;

  // Source engine pmove.c uses: accelspeed = accel * frametime * wishspeed
  float accelspeed = accel * dt * wishspeed;

  if (accelspeed > addspeed)
    accelspeed = addspeed;

  velocity.x += accelspeed * wishdir.x;
  velocity.z += accelspeed * wishdir.z;
}

void PlayerController::AirAccelerate(glm::vec3 &velocity,
                                     const glm::vec3 &wishdir, float wishspeed,
                                     float accel, float dt) {
  float currentspeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishdir);
  float addspeed = wishspeed - currentspeed;

  if (addspeed <= 0)
    return;

  // pmove.c: accelspeed = accel * wishspeed * frametime;
  float accelspeed = accel * wishspeed * dt;
  if (accelspeed > addspeed)
    accelspeed = addspeed;

  velocity.x += accelspeed * wishdir.x;
  velocity.z += accelspeed * wishdir.z;
}

// =========================================================================================

glm::vec3 PlayerController::GetVelocity() const {
  JPH::Vec3 v = m_Character->GetLinearVelocity();
  return {v.GetX(), v.GetY(), v.GetZ()};
}

void PlayerController::HandleInput(float dt) {
  m_ForwardInput = 0.0f;
  m_SideInput = 0.0f;

  if (Input::IsKeyPressed(GLFW_KEY_W))
    m_ForwardInput += 1.0f;
  if (Input::IsKeyPressed(GLFW_KEY_S))
    m_ForwardInput -= 1.0f;
  if (Input::IsKeyPressed(GLFW_KEY_A))
    m_SideInput -= 1.0f;
  if (Input::IsKeyPressed(GLFW_KEY_D))
    m_SideInput += 1.0f;

  m_JumpPressed = Input::IsKeyPressed(GLFW_KEY_SPACE);
  m_SprintPressed = Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);
  m_CrouchPressed = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL);
}

void PlayerController::UpdateCrouch(float dt) {
  if (m_CrouchPressed && !m_IsCrouching) {
    m_IsCrouching = true;
  } else if (!m_CrouchPressed && m_IsCrouching) {
    m_IsCrouching = false;
  }

  // Animate transition: move toward target (0.0 = crouched, 1.0 = standing)
  float target = m_IsCrouching ? 0.0f : 1.0f;
  if (m_CrouchTransition != target) {
    float direction = (target > m_CrouchTransition) ? 1.0f : -1.0f;
    m_CrouchTransition += direction * dt / 0.2f; // 0.2s transition
    if ((direction > 0.0f && m_CrouchTransition > target) ||
        (direction < 0.0f && m_CrouchTransition < target)) {
      m_CrouchTransition = target;
    }
  }
}

void PlayerController::UpdateSprint(float dt) {
  if (m_SprintPressed && !m_IsSprinting && m_SprintRecoveryTime <= 0.0f &&
      m_Character->GetGroundState() ==
          JPH::CharacterVirtual::EGroundState::OnGround) {
    m_IsSprinting = true;
    m_SprintRemaining = SPRINT_DURATION;
  }

  if (m_IsSprinting) {
    m_SprintRemaining -= dt;
    if (!m_SprintPressed) {
      m_IsSprinting = false;
    } else if (m_SprintRemaining <= 0.0f) {
      m_IsSprinting = false;
      m_SprintRecoveryTime = SPRINT_RECOVERY;
    }
  } else if (m_SprintRecoveryTime > 0.0f) {
    m_SprintRecoveryTime -= dt;
  }
}

glm::vec3 PlayerController::GetForwardVector(float yaw, float pitch) {
  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  return glm::normalize(front);
}

glm::vec3 PlayerController::GetRightVector(float yaw) {
  return glm::normalize(
      glm::cross(GetForwardVector(yaw, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
}

void PlayerController::SetPosition(const glm::vec3 &position) {
  m_Position = position;
  m_Character->SetPosition(JPH::RVec3(position.x, position.y, position.z));
}

void PlayerController::SetRotation(float yaw, float pitch) {
  m_Yaw = yaw;
  m_Pitch = pitch;
}

glm::vec3 PlayerController::GetPosition() const {
  JPH::RVec3 p = m_Character->GetPosition();
  return {p.GetX(), p.GetY(), p.GetZ()};
}

float PlayerController::GetSpeed() const {
  JPH::Vec3 v = m_Character->GetLinearVelocity();
  return glm::length(glm::vec2(v.GetX(), v.GetZ()));
}

} // namespace S67
