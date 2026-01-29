#include "PlayerController.h"
#include "Core/Input.h"
#include "Events/MouseEvent.h"
#include "Physics/PhysicsSystem.h"
#include "Renderer/Entity.h"
#include <GLFW/glfw3.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <utility>

namespace S67 {

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

PlayerController::PlayerController(Ref<PerspectiveCamera> camera)
    : m_Camera(camera) {

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

  m_Character = new JPH::CharacterVirtual(&settings, JPH::RVec3(0, 2, 0),
                                          JPH::Quat::sIdentity(),
                                          &PhysicsSystem::GetPhysicsSystem());
}

PlayerController::~PlayerController() {}

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

void PlayerController::OnUpdate(Timestep ts) {
  // Rotation
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

  HandleInput(ts);

  // Read current velocity from Jolt and convert to Hammer Units (HU)
  JPH::Vec3 velocityMeters = m_Character->GetLinearVelocity();
  glm::vec3 velocityHU = {velocityMeters.GetX() / HU_TO_METERS,
                          velocityMeters.GetY() / HU_TO_METERS,
                          velocityMeters.GetZ() / HU_TO_METERS};

  // 1. Check Ground State & Jump (Jump must happen before Friction to enable
  // Bhop)
  bool onGround = m_Character->GetGroundState() ==
                  JPH::CharacterVirtual::EGroundState::OnGround;
  bool justJumped = false;

  if (m_JumpPressed && onGround) {
    velocityHU.y = m_Settings.JumpVelocity;
    m_JumpPressed = false;
    justJumped = true;
    onGround = false; // Treat as air for rest of frame
  }

  // 2. Friction (Ground Only, skipped if we just jumped)
  if (onGround) {
    glm::vec3 speedVec = {velocityHU.x, 0.0f, velocityHU.z};
    float speed = glm::length(speedVec);
    if (speed > 0.01f) {
      float control = (speed < m_Settings.StopSpeed)
                          ? m_Settings.StopSpeed
                          : speed; // 100 HU/s stopspeed
      float drop = control * m_Settings.Friction * ts;
      float newSpeed = glm::max(0.0f, speed - drop);
      newSpeed /= speed;
      velocityHU.x *= newSpeed;
      velocityHU.z *= newSpeed;
    } else {
      velocityHU.x = 0.0f;
      velocityHU.z = 0.0f;
    }
  }

  // 3. Wish velocity calculation (HU)
  glm::vec3 wishDir;
  float wishSpeedHU;
  {
    glm::vec3 forward = GetForwardVector(m_Yaw, 0.0f);
    glm::vec3 right = GetRightVector(m_Yaw);
    glm::vec3 wishVel =
        (forward * m_ForwardInput * 450.0f) + (right * m_SideInput * 450.0f);

    float maxSpeed = m_Settings.MaxSpeed;
    if (m_IsSprinting && m_SprintRemaining > 0.0f)
      maxSpeed = m_Settings.MaxSprintSpeed;
    else if (m_IsCrouching)
      maxSpeed = m_Settings.MaxCrouchSpeed;

    wishSpeedHU = glm::length(wishVel);
    if (wishSpeedHU > 0.01f) {
      wishDir = glm::normalize(wishVel);
      wishSpeedHU = glm::min(wishSpeedHU, maxSpeed);
    } else {
      wishDir = glm::vec3(0.0f);
      wishSpeedHU = 0.0f;
    }
  }

  // 4. Acceleration
  if (onGround) {
    // Accelerate
    float currentSpeed = velocityHU.x * wishDir.x + velocityHU.z * wishDir.z;
    float addSpeed = wishSpeedHU - currentSpeed;
    if (addSpeed > 0.0f) {
      float accelSpeed = m_Settings.Acceleration * ts * wishSpeedHU;
      velocityHU.x += (glm::min(accelSpeed, addSpeed) * wishDir.x);
      velocityHU.z += (glm::min(accelSpeed, addSpeed) * wishDir.z);
    }
  } else {
    // Air Accelerate
    float wishSpeedCap = glm::min(wishSpeedHU, m_Settings.MaxAirWishSpeed);
    float currentSpeed = velocityHU.x * wishDir.x + velocityHU.z * wishDir.z;
    float addSpeed = wishSpeedCap - currentSpeed;
    if (addSpeed > 0.0f) {
      float accelSpeed = m_Settings.AirAcceleration * ts * wishSpeedHU;
      float finalAccel = glm::min(accelSpeed, addSpeed);
      velocityHU.x += finalAccel * wishDir.x;
      velocityHU.z += finalAccel * wishDir.z;
    }
  }

  // 5. Gravity
  if (!onGround && !justJumped) {
    velocityHU.y -= m_Settings.Gravity * ts;
  } else if (onGround) {
    velocityHU.y = -10.0f; // Small stick to ground force in HU
  }
  // If justJumped, Y is already set to JUMP_VELOCITY

  // Convert back to meters and update Jolt
  velocityMeters =
      JPH::Vec3(velocityHU.x * HU_TO_METERS, velocityHU.y * HU_TO_METERS,
                velocityHU.z * HU_TO_METERS);
  m_Character->SetLinearVelocity(velocityMeters);

  // 6. Move Character (Collision Handling)
  PlayerBodyFilter bodyFilter;
  m_Character->Update(ts, JPH::Vec3::sZero(),  // Gravity handled manually
                      PhysicsSystem::GetBroadPhaseLayerFilter(),
                      PhysicsSystem::GetObjectLayerFilter(), bodyFilter,
                      JPH::ShapeFilter(), m_TempAllocator);

  // Update Sprint & Crouch timers
  UpdateSprint(ts);
  UpdateCrouch(ts);

  // Sync Camera
  JPH::RVec3 charPos = m_Character->GetPosition();
  float eyeHeight = glm::mix(1.7f, 0.8f, 1.0f - m_CrouchTransition);
  m_Camera->SetPosition(
      {charPos.GetX(), charPos.GetY() + eyeHeight, charPos.GetZ()});
}

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

// Obsolete methods removed (consolidated into OnUpdate)

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
    // Clamp to target
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
    if (m_SprintRemaining <= 0.0f) {
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
