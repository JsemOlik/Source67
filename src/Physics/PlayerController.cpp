#include "PlayerController.h"
#include "Core/Input.h"
#include "Physics/PhysicsSystem.h"
#include "Events/MouseEvent.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <GLFW/glfw3.h>
#include "Renderer/Entity.h"

namespace S67 {

    class PlayerBodyFilter : public JPH::BodyFilter {
    public:
        virtual bool ShouldCollide(const JPH::BodyID &inBodyID) const override {
            JPH::BodyInterface& bodyInterface = PhysicsSystem::GetBodyInterface();
            uint64_t userData = bodyInterface.GetUserData(inBodyID);
            if (userData != 0) {
                Entity* entity = (Entity*)userData;
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
        settings.mShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.9f, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.9f, 0.3f)).Create().Get();
        settings.mCharacterPadding = 0.02f;
        settings.mPenetrationRecoverySpeed = 1.0f;
        settings.mPredictiveContactDistance = 0.1f;

        m_Character = new JPH::CharacterVirtual(&settings, JPH::RVec3(0, 2, 0), JPH::Quat::sIdentity(), &PhysicsSystem::GetPhysicsSystem());
    }

    PlayerController::~PlayerController() {
    }

    void PlayerController::Reset(const glm::vec3& startPos) {
        m_Position = startPos;
        SetPosition(startPos);
        m_Character->SetLinearVelocity(JPH::Vec3::sZero());
        m_Yaw = -90.0f; 
        m_Pitch = 0.0f;
        m_Camera->SetYaw(m_Yaw);
        m_Camera->SetPitch(m_Pitch);
    }

    void PlayerController::OnEvent(Event& e) {
        if (e.GetEventType() == EventType::MouseMoved) {
            MouseMovedEvent& me = (MouseMovedEvent&)e;
            if (m_FirstMouse) {
                m_LastMouseX = me.GetX();
                m_LastMouseY = me.GetY();
                m_FirstMouse = false;
            }

            float xoffset = me.GetX() - m_LastMouseX;
            float yoffset = m_LastMouseY - me.GetY(); 
            m_LastMouseX = me.GetX();
            m_LastMouseY = me.GetY();

            xoffset *= m_Sensitivity;
            yoffset *= m_Sensitivity;

            m_Yaw += xoffset; 
            m_Pitch += yoffset;

            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;

            m_Camera->SetYaw(m_Yaw);
            m_Camera->SetPitch(m_Pitch);
        }
    }

    void PlayerController::OnUpdate(Timestep ts) {
        HandleInput(ts);

        // Sync Camera
        JPH::RVec3 pos = m_Character->GetPosition();
        m_Camera->SetPosition({ pos.GetX(), pos.GetY() + 1.7f, pos.GetZ() }); // Eye height
    }

    void PlayerController::HandleInput(float dt) {
        glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
        glm::vec3 forward = m_Camera->GetForward(); 
        forward.y = 0;
        forward = glm::normalize(forward);
        glm::vec3 right = m_Camera->GetRight();
        right.y = 0;
        right = glm::normalize(right);

        if (Input::IsKeyPressed(GLFW_KEY_W)) direction += forward;
        if (Input::IsKeyPressed(GLFW_KEY_S)) direction -= forward;
        if (Input::IsKeyPressed(GLFW_KEY_A)) direction -= right;
        if (Input::IsKeyPressed(GLFW_KEY_D)) direction += right;

        float currentSpeed = Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? m_SprintSpeed : m_WalkSpeed;

        if (glm::length(direction) > 0.0f)
            direction = glm::normalize(direction);

        JPH::Vec3 currentVelocity = m_Character->GetLinearVelocity();
        glm::vec3 currentHorizontalVel = { currentVelocity.GetX(), 0.0f, currentVelocity.GetZ() };
        glm::vec3 targetHorizontalVel = direction * currentSpeed;

        float lerpFactor = (glm::length(direction) > 0.0f) ? m_Acceleration : m_Friction;
        glm::vec3 newHorizontalVel = glm::mix(currentHorizontalVel, targetHorizontalVel, std::min(dt * lerpFactor, 1.0f));

        currentVelocity.SetX(newHorizontalVel.x);
        currentVelocity.SetZ(newHorizontalVel.z);
        JPH::Vec3& velocity = currentVelocity; 

        // Gravity
        if (m_Character->GetGroundState() != JPH::CharacterVirtual::EGroundState::OnGround) {
            velocity.SetY(velocity.GetY() - 9.81f * dt * 3.0f);
        } else {
            velocity.SetY(-1.0f); // Small downward force to stay grounded
        }

        // Jump
        if (Input::IsKeyPressed(GLFW_KEY_SPACE) && m_Character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround) {
            velocity.SetY(m_JumpForce);
        }
        
        // Update Character
        JPH::TempAllocatorImpl allocator(10 * 1024 * 1024);
        PlayerBodyFilter bodyFilter;
        m_Character->Update(dt, JPH::Vec3(0, -9.81f, 0), PhysicsSystem::GetBroadPhaseLayerFilter(), PhysicsSystem::GetObjectLayerFilter(), bodyFilter, JPH::ShapeFilter(), allocator);
        
        m_Character->SetLinearVelocity(velocity);
    }

    void PlayerController::SetPosition(const glm::vec3& position) {
        m_Character->SetPosition(JPH::RVec3(position.x, position.y, position.z));
    }

    void PlayerController::SetRotation(const glm::vec3& eulerDegrees) {
        m_Yaw = eulerDegrees.y - 90.0f;
        m_Pitch = glm::clamp(eulerDegrees.x, -89.0f, 89.0f);
        
        m_Camera->SetYaw(m_Yaw);
        m_Camera->SetPitch(m_Pitch);
    }

    glm::vec3 PlayerController::GetPosition() const {
        JPH::RVec3 p = m_Character->GetPosition();
        return { p.GetX(), p.GetY(), p.GetZ() };
    }

    float PlayerController::GetSpeed() const {
        JPH::Vec3 v = m_Character->GetLinearVelocity();
        return glm::length(glm::vec2(v.GetX(), v.GetZ()));
    }

}
