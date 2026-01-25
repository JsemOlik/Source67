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
        void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

        float GetRotation() const { return m_Rotation; }
        void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

    private:
        void RecalculateViewMatrix();

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f; // Simplified rotation for now
    };

}
