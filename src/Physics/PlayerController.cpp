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

            float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

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
        glm::vec3 forward = m_Camera->GetForward(); // We use camera forward for movement direction, but flatten Y
        forward.y = 0;
        forward = glm::normalize(forward);
        glm::vec3 right = m_Camera->GetRight();
        right.y = 0;
        right = glm::normalize(right);

        if (Input::IsKeyPressed(GLFW_KEY_W)) direction += forward;
        if (Input::IsKeyPressed(GLFW_KEY_S)) direction -= forward;
        if (Input::IsKeyPressed(GLFW_KEY_A)) direction -= right;
        if (Input::IsKeyPressed(GLFW_KEY_D)) direction += right;

        if (glm::length(direction) > 0.0f)
            direction = glm::normalize(direction);

        JPH::Vec3 velocity = { direction.x * m_Speed, -9.81f, direction.z * m_Speed }; // Simple gravity
        
        // Update Character
        // We need a temp allocator for collision checks
        JPH::TempAllocatorImpl allocator(10 * 1024 * 1024);
        PlayerBodyFilter bodyFilter;
        m_Character->Update(dt, JPH::Vec3(0, -9.81f, 0), PhysicsSystem::GetBroadPhaseLayerFilter(), PhysicsSystem::GetObjectLayerFilter(), bodyFilter, JPH::ShapeFilter(), allocator);
        
        // We set linear velocity to control movement? No, CharacterVirtual is usually position-based or velocity-based but you call Update with velocity.
        // Wait, Update takes gravity. velocity is set via SetLinearVelocity usually? 
        // CharacterVirtual::SetLinearVelocity(velocity).
        
        m_Character->SetLinearVelocity(velocity);
    }

    void PlayerController::SetPosition(const glm::vec3& position) {
        m_Character->SetPosition(JPH::RVec3(position.x, position.y, position.z));
    }

    glm::vec3 PlayerController::GetPosition() const {
        JPH::RVec3 p = m_Character->GetPosition();
        return { p.GetX(), p.GetY(), p.GetZ() };
    }

}
