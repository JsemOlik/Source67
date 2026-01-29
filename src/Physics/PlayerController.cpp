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

  // 1. Update Rotation
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
  float dt = ts;

  // READ current velocity from Jolt (convert to HU/s)
  JPH::Vec3 jVelocity = m_Character->GetLinearVelocity();
  glm::vec3 velocity = {jVelocity.GetX() / HU_TO_METERS,
                        jVelocity.GetY() / HU_TO_METERS,
                        jVelocity.GetZ() / HU_TO_METERS};

  // 1. Check if grounded
  CheckGround();

  // 2. Queue Jump (if pressed and grounded) - Standard Source/Quake Logic
  // In Quake command tick, jump sets velocity.z (up) and unsets onground flag
  // logic is usually: CheckGround() -> Friction() -> WalkMove/AirMove ->
  // CheckGround() (Categorize Position) But for better responsiveness we can
  // handle the jump "wish" here. Actually, Quake applies jump *inside*
  // CheckJump/PlayerMove, modifying velocity immediatelly. If we jump, we MUST
  // NOT apply friction this frame (or friction applies to the jump takeoff,
  // which is wrong).

  bool queueJump = false;
  if (m_JumpPressed && m_OnGround) {
    queueJump = true;
    m_OnGround = false; // We are launching now

    // Apply Jump Velocity
    // Source preserves XZ velocity on jump (no friction applied this tick if we
    // jump)
    velocity.y = m_Settings.JumpVelocity;
  }

  // 3. Friction (only if on ground and didn't just jump)
  if (m_OnGround) {
    ApplyFriction(velocity, dt);
  }

  // 4. Calculate Wish Direction & Speed
  glm::vec3 forward = GetForwardVector(m_Yaw, 0.0f);
  glm::vec3 right = GetRightVector(m_Yaw);
  glm::vec3 wishVel =
      (forward * m_ForwardInput * 450.0f) + (right * m_SideInput * 450.0f);

  float wishSpeed = glm::length(wishVel);
  glm::vec3 wishDir =
      (wishSpeed > 0.0f) ? glm::normalize(wishVel) : glm::vec3(0.0f);

  // Cap wishSpeed to MaxSpeed types
  float currentMaxSpeed = m_Settings.MaxSpeed;
  if (m_IsSprinting && m_SprintRemaining > 0.0f)
    currentMaxSpeed = m_Settings.MaxSprintSpeed;
  else if (m_IsCrouching)
    currentMaxSpeed = m_Settings.MaxCrouchSpeed;

  if (wishSpeed > currentMaxSpeed) {
    wishSpeed = currentMaxSpeed;
  }

  // 5. Accelerate
  if (m_OnGround) {
    // Walk Movement
    // Source: WalkMove calls Accelerate
    Accelerate(velocity, wishDir, wishSpeed, m_Settings.Acceleration, dt);

    // Apply "gravity" for staying on slopes/stairs (snap to ground)?
    // Jolt Character handles slope sliding, but we might want a small downward
    // push to keep contact
    velocity.y = 0.0f; // Reset Y velocity while walking
  } else {
    // Air Movement
    // Source: AirMove calls AirAccelerate
    // Standard Quake/Source AirAccelerate limits wishspeed to 30 for strafing
    // logic
    float airWishSpeed = wishSpeed;
    if (airWishSpeed > m_Settings.MaxAirWishSpeed) {
      airWishSpeed = m_Settings.MaxAirWishSpeed;
    }
    AirAccelerate(velocity, wishDir, airWishSpeed, m_Settings.AirAcceleration,
                  dt);

    // Apple Gravity
    velocity.y -= m_Settings.Gravity * dt;
  }

  // WRITE velocity back to Jolt (Meters)
  JPH::Vec3 newJVelocity = {velocity.x * HU_TO_METERS,
                            velocity.y * HU_TO_METERS,
                            velocity.z * HU_TO_METERS};
  m_Character->SetLinearVelocity(newJVelocity);

  // 6. Physics Step
  PlayerBodyFilter bodyFilter;
  m_Character->Update(ts, JPH::Vec3::sZero(), // We handle gravity manually
                      PhysicsSystem::GetBroadPhaseLayerFilter(),
                      PhysicsSystem::GetObjectLayerFilter(), bodyFilter,
                      JPH::ShapeFilter(), m_TempAllocator);

  // Update Timers
  UpdateSprint(ts);
  UpdateCrouch(ts);

  // Reset Jump Input (processed)
  m_JumpPressed = false;

  // Sync Camera
  JPH::RVec3 charPos = m_Character->GetPosition();
  float eyeHeight =
      glm::mix(1.7f * (1.0f / 0.0254f * 0.01905f),
               0.8f * (1.0f / 0.0254f * 0.01905f), 1.0f - m_CrouchTransition);
  // Wait, eyeHeight 1.7m is standard metric. 1.7m = 64 units approx in Quake?
  // Quake ViewHeight: Standing 64 units, Crouching 28 units.
  // 64 units * 0.01905 = 1.2192 meters.
  // Existing code had 1.7f meters which is ~1.7/0.01905 = 89 units.
  // Source default player height is 72 units (1.37m at 0.75 scale). View
  // height 64. Let's stick roughly to what was there or map to Source units
  // correctly. Let's use Source values: 64 units view height.
  eyeHeight = glm::mix(64.0f * HU_TO_METERS, 28.0f * HU_TO_METERS,
                       1.0f - m_CrouchTransition);

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

    // Stop sprinting if shift key is released
    if (!m_SprintPressed) {
      m_IsSprinting = false;
      // No recovery penalty if manually stopped
    }
    // Or if sprint duration runs out (exhausted)
    else if (m_SprintRemaining <= 0.0f) {
      m_IsSprinting = false;
      m_SprintRecoveryTime = SPRINT_RECOVERY; // Only penalize if exhausted
    }
  } else if (m_SprintRecoveryTime > 0.0f) {
    m_SprintRecoveryTime -= dt;
  }
}

void PlayerController::CheckGround() {
  JPH::CharacterVirtual::EGroundState groundState =
      m_Character->GetGroundState();
  m_OnGround = (groundState == JPH::CharacterVirtual::EGroundState::OnGround);

  // Update Jolt about our "Up" vector? It assumes Y up.
}

void PlayerController::ApplyFriction(glm::vec3 &velocity, float dt) {
  glm::vec3 speedVec = {velocity.x, 0.0f, velocity.z};
  float speed = glm::length(speedVec);

  if (speed < 0.1f)
    return;

  float control = (speed < m_Settings.StopSpeed) ? m_Settings.StopSpeed : speed;
  float drop = control * m_Settings.Friction * dt;

  float newSpeed = speed - drop;
  if (newSpeed < 0)
    newSpeed = 0;

  if (speed > 0)
    newSpeed /= speed;

  velocity.x *= newSpeed;
  velocity.z *= newSpeed;
}

void PlayerController::Accelerate(glm::vec3 &velocity, const glm::vec3 &wishDir,
                                  float wishSpeed, float accel, float dt) {
  float currentSpeed = glm::dot(velocity, wishDir);
  float addSpeed = wishSpeed - currentSpeed;

  if (addSpeed <= 0)
    return;

  float accelSpeed = accel * dt * wishSpeed * m_Settings.Friction;
  // Wait, Source `Accelerate` uses `wishSpeed`?
  // Source: accelspeed = accel * dt * wishspeed * surfaceFriction;
  // If we are AirAccelerating, surfaceFriction is usually 1?
  // Actually Source AirAccelerate is diff function.

  // Standard Quake Accelerate:
  // if (!onGround) return; // handled by caller

  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  velocity.x += accelSpeed * wishDir.x;
  velocity.z += accelSpeed * wishDir.z;
}

void PlayerController::AirAccelerate(glm::vec3 &velocity,
                                     const glm::vec3 &wishDir, float wishSpeed,
                                     float accel, float dt) {
  float currentSpeed = glm::dot(velocity, wishDir);
  float addSpeed = wishSpeed - currentSpeed;

  if (addSpeed <= 0)
    return;

  float accelSpeed = accel * wishSpeed * dt;
  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  velocity.x += accelSpeed * wishDir.x;
  velocity.z += accelSpeed * wishDir.z;
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
  // Return meters? Or HU?
  // The rest of the engine seems to be using meters (Rendering usually).
  // But our logical position.. well, keeping it meters for now as it comes from
  // Jolt.
  return {p.GetX(), p.GetY(), p.GetZ()};
}

float PlayerController::GetSpeed() const {
  JPH::Vec3 v = m_Character->GetLinearVelocity();
  glm::vec3 velocity = {v.GetX() / HU_TO_METERS, v.GetY() / HU_TO_METERS,
                        v.GetZ() / HU_TO_METERS};
  return glm::length(glm::vec2(velocity.x, velocity.z));
}

} // namespace S67
