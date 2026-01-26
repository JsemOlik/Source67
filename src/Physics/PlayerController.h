#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/Camera.h"
#include "Events/Event.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <glm/glm.hpp>

namespace S67 {

    class PlayerController {
    public:
        PlayerController(Ref<PerspectiveCamera> camera);
        ~PlayerController();

        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);
        void Reset(const glm::vec3& startPos);
        
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& eulerDegrees); // Added
        glm::vec3 GetPosition() const;
        float GetSpeed() const;

        void SetWalkSpeed(float speed) { m_WalkSpeed = speed; }
        void SetSprintSpeed(float speed) { m_SprintSpeed = speed; }
        void SetSensitivity(float sensitivity) { m_Sensitivity = sensitivity; }
        void SetFOV(float fov) { m_FOV = fov; m_Camera->SetProjection(fov, m_Camera->GetAspectRatio(), 0.1f, 1000.0f); }

    private:
        void HandleInput(float dt);

    private:
        Ref<PerspectiveCamera> m_Camera;
        JPH::Ref<JPH::CharacterVirtual> m_Character;
        glm::vec3 m_Position = { 0.0f, 2.0f, 0.0f };
        float m_WalkSpeed = 6.0f;
        float m_SprintSpeed = 10.0f;
        float m_JumpForce = 8.0f; 
        float m_Acceleration = 30.0f;
        float m_Friction = 15.0f;
        float m_Sensitivity = 0.1f;
        float m_FOV = 45.0f;
        
        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
        float m_Pitch = 0.0f, m_Yaw = -90.0f; // Default yaw facing -Z
    };

}
