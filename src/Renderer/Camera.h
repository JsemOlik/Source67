#pragma once

#include <glm/glm.hpp>

namespace S67 {

    class Camera {
    public:
        virtual ~Camera() = default;

        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    protected:
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);
    };

    class PerspectiveCamera : public Camera {
    public:
        PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void SetProjection(float fov, float aspectRatio, float nearClip, float farClip);

        const glm::vec3& GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& position) { m_Position = position; UpdateViewMatrix(); }

        const glm::vec3& GetForward() const { return m_Front; }
        const glm::vec3& GetRight() const { return m_Right; }

        float GetYaw() const { return m_Yaw; }
        void SetYaw(float yaw) { m_Yaw = yaw; UpdateViewMatrix(); }

        float GetPitch() const { return m_Pitch; }
        void SetPitch(float pitch) { m_Pitch = pitch; UpdateViewMatrix(); }

        void UpdateViewMatrix();

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
        glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
        glm::vec3 m_Right;
        glm::vec3 m_WorldUp = { 0.0f, 1.0f, 0.0f };

        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;
    };

}
