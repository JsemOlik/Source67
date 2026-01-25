#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace S67 {

    PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip) {
        SetProjection(fov, aspectRatio, nearClip, farClip);
        m_ViewMatrix = glm::mat4(1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::SetProjection(float fov, float aspectRatio, float nearClip, float farClip) {
        m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::RecalculateViewMatrix() {
        // Simple camera tracking for now (improving this later with quaternions/Euler)
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
                              glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 1, 0));

        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

}
